#include "include/syscall.h"

void bp_term_proc_non_corrente(){}
void bp_term_current_proc(){}
void bp_term_figlio_not_found(){}
void bp_term_ho_processi_figli(){}
void bp_term_proc_in_semd(){}
void bp_term_elimino_padre(){}

void bp_type_SYS(){}
void bp_type_TLB(){}
void bp_type_TRAP(){}

//SYSCALL 1
void getCPUTime(unsigned int *user, unsigned int *kernel, unsigned int *wallclock){
    
    #ifdef TARGET_UMPS
        
        // Incremento il tempo totale del kernel
        // ACTIVE_PCB->kernel_total += getTODLO()-ACTIVE_PCB->kernel_total;

        
        //Incremento il tempo totale del kernel
        ACTIVE_PCB->kernel_total = ACTIVE_PCB->kernel_total + (getTODLO()-ACTIVE_PCB->kernel_start);
        
        //Incremento il tempo totale dello user
        //ACTIVE_PCB->user_total = ACTIVE_PCB->user_total + getTODLO()-ACTIVE_PCB->user_start;
        
            
        //Se ho un tempo user lo assegno 
        if(user != NULL){

            *(user) = ACTIVE_PCB->user_total;
            
        }
        
        //Se ho un tempo kernel lo assegno 
        if(kernel != NULL){
            
            *(kernel) = ACTIVE_PCB->kernel_total;
        
        }
            
        //Se ho un tempo totale lo assegno 
        if(wallclock != NULL){
        
            *(wallclock) = getTODLO() - ACTIVE_PCB->wallclock_start;
        
        }

    #endif
    
}

//SYSCALL 2
int CreateProcess(state_t *statep, int priority, void ** cpid){

    //Creo nuovo processo figlio
    pcb_t* tempPcb = allocPcb();
    
    // Controllo che l'indirizzo sia corretto
    //*((pcb_t **)cpid) = tempPcb;
   

    if (tempPcb != NULL){ //Ha successo: cpid non NULL e tempPcb allocato correttamente

        //Assegno lo stato del nuovo processo figlio
        SavePCBToOldArea(statep, &(tempPcb->p_s));

        //Settiamo la priority
        tempPcb->priority = priority;
        tempPcb->original_priority = priority;

        //Inserisco tempPcb come figlio di ACTIVE_PCB
        insertChild(ACTIVE_PCB, tempPcb);

        //Inserisco il figlio nella ready queue
        insertProcQ(ready_queue, tempPcb);

        //Se cpid non è vuoto lo aggiorno con il nuovo processo filgio istanziato, altrimenti lo lascia NULL
        if(cpid != NULL){

            *((pcb_t **)cpid) = tempPcb;
            
        }

        return 0;
        
    }
			
    else{ //Non ha successo

        return -1;
            
    }
    
}

//SYSCALL 3
int TerminateProcess(void * pid){

    // Int SYSCALL(TERMINATEPROCESS, void * pid, 0, 0)
    // Quando invocata, la SYS3 termina il processo
    // identificato da pid (il proc. corrente se pid ==
    // NULL) insieme alla sua progenie. pid e’ un
    // puntatore a pcb_t cosi’ come viene inizializzato
    // dalla system call Create_Process
    // – Restituisce 0 se ha successo, -1 per errore (e.g. il
    // pid non corrisponde a un processo esistente).

    //Questo è il processo da terminare
    pcb_t *tempPcb;

    //Il processo da eliminare è quello corrente 
    if(pid == NULL || pid == 0){

        

        //Dico che il processo da eliminare è il corrente
        tempPcb = ACTIVE_PCB;

    }

    //Il processo da eliminare non è il corrente
    else{

        bp_term_proc_non_corrente();

        tempPcb = pid;

        if(tempPcb->p_parent==NULL){
            bp_term_current_proc();
        }

        //Controllo se tempPCB è nella progenie di ACTIVEPCB, se non fa parte dela progenie è errore
        if( !isChild(ACTIVE_PCB, tempPcb) || (pid == NULL) ){
            
            bp_term_figlio_not_found();
            
            return -1;

        }

        //Rimuove il PCB puntato da p dalla lista dei figli del padre
        //outChild(tempPcb);
        
    }

    pcb_t *figlio;

    //caso ricorsivo: tempPCB ha dei figli - chiamo la TerminateProcess sui figli
    //Controllo se il processo da eliminare ha figli
    if( !emptyChild(tempPcb) ){

        //Elimino la progenie del tempPcb
        //figlio = removeChildNonOrfano(tempPcb);
        figlio = returnFirstChild(tempPcb);

        while(figlio != NULL){
            //Rimuove il PCB puntato da p dalla lista dei figli del padre
            TerminateProcess(figlio);

            figlio = returnFirstChild(tempPcb);
        }
    
    }
    
    //caso base: tempPCB è una foglia (non ha figli da eliminare - non deve andare in ricorsione)
    else {

        //settare il padre del tempPCB = NULL
        //togliere il tempPCB dalla lista dei padre->p_figli
        outChild(tempPcb);

    }

    


    //Controllo se il processo da eliminare è bloccato su un semaforo
    if(tempPcb->p_semkey != NULL){

        bp_term_proc_in_semd();

        //Prendo il semaforo sulla quale è bloccato il processo
        int *semaforo = tempPcb->p_semkey;

        //Sblocco il processo dal semaforo
        outBlocked(tempPcb);
        
        //Aggiorno il numero dei processi bloccati su quel semaforo
        *(semaforo)+=1;

        //Aggiorno il contatore
        BLOCK_COUNT--;

    }

    //Rimuovo il processo dalla ready queue
    outProcQ(ready_queue,tempPcb);

    //Controllo se il processo da eliminare è quello attivo
    if(tempPcb == ACTIVE_PCB){

        //Setto il processo attivo a NULL
        ACTIVE_PCB = NULL;
    
    }

tempPcb->p_parent = NULL;

    freePcb(tempPcb);

    return 0;
    
}

//SYSCALL 4 - risveglia il rocesso dall'attesa
void Verhogen(int *semaddr){

    //Incremento il semaforo
    *semaddr+=1;

    //Prendo il primo processo messo in attesa - ritorna NULL se non ci sono processi
    pcb_t* pcb_blocked = removeBlocked(semaddr);   

	// Controllo se ho più thread nella lista d'attesa
	if (*semaddr <= 0){     
         
        if(pcb_blocked!=NULL){
                        
            //Setto la chiave del semaforo su cui il PCB è bloccato a NULL
            pcb_blocked->p_semkey = NULL;

            //Aggiorno il contatore dei processi bloccati
            BLOCK_COUNT--;
            
            //Inserisco il processo nella ready queue
            insertProcQ(ready_queue, pcb_blocked);
            
        }
        
    }

}

//SYSCALL 5 - metto processo in attesa su un s
void Passeren(int *semaddr){

    //Decremento il semaforo
    *semaddr-=1;

    //Controllo se ci sono altri processi bloccati
    if (*semaddr < 0){
        
        //Salvo i registri dell'old area della sys al processo 
        state_t* oldarea = ((state_t*)SYSBK_OLDAREA);

        //Copio lo stato della old area della sys nel processo che lo ha sollevato 
        SavePCBToOldArea(oldarea, &(ACTIVE_PCB->p_s));
        
        #ifdef TARGET_UMPS
        
            //Salvo il valore del tempo in kernelmode
            if(ACTIVE_PCB->kernel_start > 0){

                ACTIVE_PCB->kernel_total += getTODLO() - ACTIVE_PCB->kernel_start;
                ACTIVE_PCB->kernel_start = 0;

            }
        
        #endif 
        
        //Metto il processo nella coda del semaforo
	 	int ret = insertBlocked(semaddr, ACTIVE_PCB);
        
        //Aggiorno il contatore dei processi bloccati
        BLOCK_COUNT ++;
        
        // L'ACTIVE PCB VA MESSO A NULL ALLA FINE DELLA SYSCALL PRIMA DI CHIAMARE LO SCHEDULER
        ACTIVE_PCB = NULL;

        //assegnamento al semd NON andato a buon fine
        if(ret){

            PANIC();
        
        }
     
    }    

}

//SYSCALL 6
int DO_IO(unsigned int command, unsigned int* registro, int subdevice){

    int *sem;
    int dev = 0;
    int line = 0;
    int status = 0;

    dev = numDev((unsigned int *)registro);
    line = numLine((unsigned int *)registro);

    //Non è un terminale
    if(line < 7){
        
        //Blocco il processo 
        if(line == 3)        sem = Semaforo.disk[dev].s_key;
        else if(line == 4)    sem = Semaforo.tape[dev].s_key;
        else if(line == 5)    sem = Semaforo.network[dev].s_key;
        else if(line == 6)    sem = Semaforo.printer[dev].s_key;
        
        dtpreg_t *devreg = (dtpreg_t *) registro;

        if(!*sem){

            devreg->command = command;

        }
        
        status = devreg->status;
        //termprint ("Setto lo status 1 \n");

    }

    //E' un terminale
    else{

        termreg_t *termreg = (termreg_t*) registro;

        //Trasmissione
        if(subdevice == FALSE){

            //Blocco il processo 
            sem = Semaforo.terminalT[dev].s_key;

            
            //La prima volta che entra inuna DoIO il dispositivo è ready, in quel caso non prendo il comando 
            if(!*sem){ 
                
                //Entra qua se il contenuto di sem è 0, ovvero se non ci sono processi bloccati su quel semaforo

                termreg->transm_command = command;
            
            }
         

            status = termreg->transm_status;

        }

        //Ricezione
        else{

            //Blocco il processo 
            sem = Semaforo.terminalR[dev].s_key;
            
            if(!*sem){ 
                
                //Entra qua se il contenuto di sem è 0, ovvero se non ci sono processi bloccati su quel semaforo

                termreg->recv_command = command;
         
            }
     

            status = termreg->recv_status;
                                   
        }
       
    }

    //Qua sto venendo da una sys
    if(GOODMORNING_PCB == ACTIVE_PCB){
        
        Passeren(sem);
        //Dopo la passeren l'active pcb è a null, gli riassegno il processo svegliato

    }
             
    //Qua vengo da un interrupt 
    else{

        Passeren(sem);
        outProcQ(ready_queue, GOODMORNING_PCB);        
        
    }

    //Qua lo status è BUSY
    
    return status;
}

//SYSCALL 7
int SpecPassup(int type, state_t *old, state_t *nuovo){

    // int SYSCALL(SPECPASSUP, int type, state_t *old, state_t *new)
    // – Questa chiamata registra quale handler di livello superiore
    // debba essere attivato in caso di trap di Syscall/breakpoint
    // (type=0), TLB (type=1) o Program trap (type=2). Il significato
    // dei parametri old e new è lo stesso delle aree old e new
    // gestite dal codice della ROM: quando avviene una trap da
    // passare al gestore lo stato del processo che ha causato la
    // trap viene posto nell’area old e viene caricato o stato
    // presente nell’area new. La system call deve essere
    // richiamata una sola volta per tipo (pena la terminazione). Se
    // la system call ha successo restituisce 0, altrimenti -1.
/*
    if((ACTIVE_PCB->SysOld != NULL || ACTIVE_PCB->SysNew != NULL) && type == 0)
		return -1;

	if((ACTIVE_PCB->TLBOld != NULL || ACTIVE_PCB->TLBNew != NULL) && type == 1)
		return -1;

	if((ACTIVE_PCB->PTOld != NULL || ACTIVE_PCB->PTOld != NULL) && type == 2)
		return -1;

	switch(type){
		case 0:

			//caso syscall/breakpoint
			ACTIVE_PCB->SysOld = old;
			ACTIVE_PCB->SysNew = nuovo;
			return 0;

			break;

		case 1:

			//caso TLB
			ACTIVE_PCB->TLBOld = old;
			ACTIVE_PCB->TLBNew = nuovo;
			return 0;

			break;

		case 2:

			//caso Program Trap
			ACTIVE_PCB->PTOld = old;
			ACTIVE_PCB->PTNew = nuovo;
			return 0;

			break;

		default:

			return -1;

			break;
	}
*/

    //Se devo assegnare l'handler del livello superiore di una Sys o Bp e le aree relative non sono ancora state settate nel PCB (quindi è la prima volta che le setto) allora le assegno
    if(type == 0 && ACTIVE_PCB->SysOld == NULL && ACTIVE_PCB->SysNew == NULL ){
        
        //bp_type_SYS();

        //Assegno le aree
        ACTIVE_PCB->SysOld = old;
        ACTIVE_PCB->SysNew = nuovo;
        
        return 0;

    }

    //Se devo assegnare l'handler del livello superiore di una TLB e le aree relative non sono ancora state settate nel PCB (quindi è la prima volta che le setto) allora le assegno
    if(type == 1 && ACTIVE_PCB->TLBOld == NULL && ACTIVE_PCB->TLBNew == NULL ){
        
        //bp_type_TLB();

        //Assegno le aree
        ACTIVE_PCB->TLBOld = old;
        ACTIVE_PCB->TLBNew = nuovo;
        
        return 0;

    }

    //Se devo assegnare l'handler del livello superiore di una Program trap e le aree relative non sono ancora state settate nel PCB (quindi è la prima volta che le setto) allora le assegno
    if(type == 2 && ACTIVE_PCB->PTOld == NULL && ACTIVE_PCB->PTNew == NULL ){
        
        //bp_type_TRAP();

        //Assegno le aree
        ACTIVE_PCB->PTOld = old;
        ACTIVE_PCB->PTNew = nuovo;
        
        return 0;

    }

    //Nessuno di questi: errore 
    else{

        return -1;

    }

}

//SYSCALL 8
void getPid(void ** pid, void ** ppid){

    // Void SYSCALL(GETPID, void ** pid, void ** ppid, 0)
    // – Questa system call assegna l’identificativo del
    // processo corrente a *pid (se pid != NULL) e
    // l’identificativo del processo genitore a *ppid (se
    // ppid != NULL)
    
    //Controllo se pid non è null allora faccio il dovuto assegnamento 
    if (pid != NULL){

        // Assegna l’identificativo del processo corrente a *pid
        *pid = ACTIVE_PCB;

    }

    //assegna l'identificativo del processo genitore a *ppid
    if (ppid != NULL){
        
        //Assegno l'id del genitore
        *ppid = ACTIVE_PCB->p_parent;
        
    }

}