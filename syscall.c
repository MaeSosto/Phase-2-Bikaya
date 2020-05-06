#include "include/syscall.h"

//SYSCALL 1
void getCPUTime(unsigned int *user, unsigned int *kernel, unsigned int *wallclock){
    
    //Incremento il tempo totale del kernel
    ACTIVE_PCB->kernel_total = ACTIVE_PCB->kernel_total + getTODLO()-ACTIVE_PCB->kernel_start;
    
    //Incremento il tempo totale dello user
    ACTIVE_PCB->user_total = ACTIVE_PCB->user_total + getTODLO()-ACTIVE_PCB->user_start;
    
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
    
}

//SYSCALL 2
int CreateProcess(state_t *statep, int priority, void ** cpid){

    //Creo nuovo processo figlio
    pcb_t* tempPcb = allocPcb();
    
    // Controllo che l'indirizzo sia corretto
    //*((pcb_t **)cpid) = tempPcb;
   

    if (tempPcb != NULL){ //Ha successo: cpid non NULL e tempPcb allocato correttamente

        //Assegno lo stato del nuovo processo figlio
        SaveOldState(statep, &(tempPcb->p_s));

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

    // L'ACTIVE PCB VA MESSO A NULL ALLA FINE DELLA SYSCALL


    // Int SYSCALL(TERMINATEPROCESS, void * pid, 0, 0)
    // Quando invocata, la SYS3 termina il processo
    // identificato da pid (il proc. corrente se pid ==
    // NULL) insieme alla sua progenie. pid e’ un
    // puntatore a pcb_t cosi’ come viene inizializzato
    // dalla system call Create_Process
    // – Restituisce 0 se ha successo, -1 per errore (e.g. il
    // pid non corrisponde a un processo esistente).

    struct pcb_t *tempPcb;

    tempPcb = pid;

    if(!tempPcb){
        
        return -1;

    }

    else{

        //Se il processo che considero è uguale a null allora termino il processo corrente
        if(pid == NULL){
            
            tempPcb = ACTIVE_PCB;
            ACTIVE_PCB = NULL;
            
        }

        //Rimuovo la progenie
        outChildBlocked(tempPcb); 

        //Termino il processo e lo libero rimuovendolo dalla ready queue
        freePcb(tempPcb); 

        ACTIVE_PCB = NULL;
        
        return 0;
    }

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
        state_t* oldarea = ((state_t*)SYSCALL_OLDAREA);

        //Copio lo stato della old area della sys nel processo che lo ha sollevato 
        SaveOldState(oldarea, &(ACTIVE_PCB->p_s));
        
        //Salvo il valore del tempo in kernelmode

	    ACTIVE_PCB->kernel_total += getTODLO() - ACTIVE_PCB->kernel_start;
        ACTIVE_PCB->kernel_start=0;

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
int SpecPassup(int type, struct state *old, struct state *nuovo){

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

    


    
    
    return 0;
}

//SYSCALL 8
void getPid(void ** pid, void ** ppid){

    // Void SYSCALL(GETPID, void ** pid, void ** ppid, 0)
    // – Questa system call assegna l’identificativo del
    // processo corrente a *pid (se pid != NULL) e
    // l’identificativo del processo genitore a *ppid (se
    // ppid != NULL)
    
    // Assegna l’identificativo del processo corrente a *pid
    if (pid != NULL){

        *((pcb_t **)pid) = ACTIVE_PCB;

        //assegna l'identificativo del processo genitore a *ppid
        if (ppid != NULL){

            if (ACTIVE_PCB->p_parent != NULL){
            
                *((pcb_t **)ppid) = ACTIVE_PCB->p_parent;
            
            }

        }

    }

}