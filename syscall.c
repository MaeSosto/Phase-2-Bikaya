#include "include/syscall.h"

//SYSCALL 1
void getCPUTime(unsigned int *user, unsigned int *kernel, unsigned int *wallclock){  

    //Aggiorno il tempo passato in kernel e in user mode
    stopKernelTime(ACTIVE_PCB);
    stopUserTime(ACTIVE_PCB);
        
    //Assegno i valori ai registri
    *user = ACTIVE_PCB->user_total;

    *kernel = ACTIVE_PCB->kernel_total;

    *wallclock = getTODLO() - ACTIVE_PCB->wallclock_start;

    //Riprendo in kernel mode
    startKernelTime(ACTIVE_PCB);

}

//SYSCALL 2
int CreateProcess(state_t *statep, int priority, void ** cpid){

    //Creo nuovo processo figlio
    pcb_t* tempPcb = allocPcb();
   
    //TempPcb allocato correttamente
    if (tempPcb != NULL){ 

        //Assegno lo stato del nuovo processo figlio
        SaveState(statep, &(tempPcb->p_s));

        //Settiamo la priority
        tempPcb->priority = priority;

        tempPcb->original_priority = priority;

        //Inserisco tempPcb come figlio di ACTIVE_PCB
        insertChild(ACTIVE_PCB, tempPcb);

        //Inserisco il figlio nella ready queue
        insertProcQ(ready_queue, tempPcb);

        //Aggiorno cpid con il nuovo processo figlio istanziato
        if(cpid != NULL){

            *((pcb_t **)cpid) = tempPcb;
            
        }

        return 0;
        
    }
	
    //TempPcb non allocato correttamente, la pcbFree è vuota
    else{ 

        return -1;
            
    }
    
}

//SYSCALL 3
int TerminateProcess(void * pid){

    //Questo è il processo da terminare
    pcb_t *tempPcb;

    //Il processo da eliminare è quello corrente 
    if(pid == NULL || pid == 0){

        //Dico che il processo da eliminare è il corrente
        tempPcb = ACTIVE_PCB;

    }

    //Il processo da eliminare non è il corrente
    else{

        tempPcb = pid;

        //Controllo se tempPCB è nella progenie di ACTIVEPCB
        if( !isChild(ACTIVE_PCB, tempPcb) || (pid == NULL) ){
            
            return -1;

        }
        
    }

    pcb_t *figlio;

    //Caso ricorsivo: tempPCB ha dei figli, chiamo la TerminateProcess sui figli
    if( !emptyChild(tempPcb) ){

        //Elimino la progenie del tempPcb
        figlio = returnFirstChild(tempPcb);

        while(figlio != NULL){
            
            //Chiamo ricorsivamente la TerminateProcess
            TerminateProcess(figlio);
            
            //Rimuove il PCB puntato da p dalla lista dei figli del padre
            figlio = returnFirstChild(tempPcb);
        
        }
    
    }
    
    //Caso base: tempPCB è una foglia (non ha figli da eliminare, non deve andare in ricorsione)
    else {

        //Togliere il tempPCB dalla lista dei padre->p_figli
        outChild(tempPcb);

    }

    //Controllo se il processo da eliminare è bloccato su un semaforo
    if(tempPcb->p_semkey != NULL){

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

        //Gestione del tempo
        stopKernelTime(ACTIVE_PCB);
        
        //Setto il processo attivo a NULL
        ACTIVE_PCB = NULL;
    
    }

    //Rimetto il pcb nella pcbFree
    freePcb(tempPcb);

    return 0;
    
}

//SYSCALL 4
void Verhogen(int *semaddr){

    //Incremento il semaforo
    *semaddr+=1;

    //Prendo il primo processo messo in attesa, ritorna NULL se non ci sono processi
    pcb_t* pcb_blocked = removeBlocked(semaddr);

	//Controllo se ho più pcb nella lista d'attesa
	if (*semaddr <= 0){     
         
        if(pcb_blocked!=NULL){
                        
            //Setto la chiave del semaforo su cui il pcb è bloccato a NULL
            pcb_blocked->p_semkey = NULL;

            //Aggiorno il contatore dei processi bloccati
            BLOCK_COUNT--;
            
            //Inserisco il processo nella ready queue
            insertProcQ(ready_queue, pcb_blocked);
            
        }
        
    }

}

//SYSCALL 5 
void Passeren(int *semaddr){

    //Decremento il semaforo
    *semaddr-=1;

    //Controllo se ci sono altri processi bloccati
    if (*semaddr < 0){
        
        //Salvo i registri dell'old area della sys al processo
        state_t* oldarea = ((state_t*)SYSBK_OLDAREA);

        //Copio lo stato della old area della sys nel processo che lo ha sollevato 
        SaveState(oldarea, &(ACTIVE_PCB->p_s));
        
        //Gestione del tempo
        stopKernelTime(ACTIVE_PCB);
              
        //Metto il processo nella coda del semaforo
	 	int ret = insertBlocked(semaddr, ACTIVE_PCB);
        
        //Aggiorno il contatore dei processi bloccati
        BLOCK_COUNT ++;
        
        //Il pcb viene settato a null per effettuare effettuare lo scheduling
        ACTIVE_PCB = NULL;

        //Assegnamento al semd NON andato a buon fine
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

    //Recupero numero e linea del device
    dev = numDev((unsigned int *)registro);
    line = numLine((unsigned int *)registro);

    //Il device non è un terminale
    if(line < 7){
        
        //Blocco il processo 
        if(line == 3)        sem = Semaforo.disk[dev].s_key;
        else if(line == 4)    sem = Semaforo.tape[dev].s_key;
        else if(line == 5)    sem = Semaforo.network[dev].s_key;
        else if(line == 6)    sem = Semaforo.printer[dev].s_key;
        
        dtpreg_t *devreg = (dtpreg_t *) registro;

        //Copio il parametro command nel campo comando del registro del dispositivo
        if(!*sem){

            devreg->command = command;

        }
        
        //Assegno lo status
        status = devreg->status;

    }

    //Il device è un terminale
    else{

        termreg_t *termreg = (termreg_t*) registro;

        //Terminale in trasmissione
        if(subdevice == FALSE){

            //Prendo il pcb in attesa sul semaforo
            sem = Semaforo.terminalT[dev].s_key;

            //Entra se il contenuto di sem è 0, ovvero se non ci sono processi bloccati su quel semaforo (il dispositivo è ready)
            if(!*sem){ 
                
                termreg->transm_command = command;
            
            }

            //Assegno lo status
            status = termreg->transm_status;

        }

        //Terminale in ricezione
        else{

            //Prendo il pcb in attesa sul semaforo
            sem = Semaforo.terminalR[dev].s_key;
            
            //Entra se il contenuto di sem è 0, se non ci sono processi bloccati su quel semaforo (il dispositivo è ready)
            if(!*sem){ 
                
                termreg->recv_command = command;
         
            }

            //Assegno lo status
            status = termreg->recv_status;
                                   
        }
       
    }

    //Blocco il processo
    Passeren(sem);

    return status;

}

//SYSCALL 7
int SpecPassup(int type, state_t *old, state_t *nuovo){
    
    //Assegno l'handler del livello superiore di una sys o bp nelle aree del Pcb, se non è mai stato assegnato
    if(type == 0 && ACTIVE_PCB->SysOld == NULL && ACTIVE_PCB->SysNew == NULL ){

        //Assegno le aree
        ACTIVE_PCB->SysOld = old;
        ACTIVE_PCB->SysNew = nuovo;
        
        return 0;

    }

    //Assegno l'handler del livello superiore di una tlb nelle aree del Pcb, se non è mai stato assegnato
    if(type == 1 && ACTIVE_PCB->TLBOld == NULL && ACTIVE_PCB->TLBNew == NULL ){        

        //Assegno le aree
        ACTIVE_PCB->TLBOld = old;
        ACTIVE_PCB->TLBNew = nuovo;
        
        return 0;

    }

    //Assegno l'handler del livello superiore di una trap nelle aree del Pcb, se non è mai stato assegnato
    if(type == 2 && ACTIVE_PCB->PTOld == NULL && ACTIVE_PCB->PTNew == NULL ){

        //Assegno le aree
        ACTIVE_PCB->PTOld = old;
        ACTIVE_PCB->PTNew = nuovo;

        return 0;

    }

    //Il tipo di exception non è riconosciuto
    else{

        return -1;

    }

}

//SYSCALL 8
void getPid(void ** pid, void ** ppid){

    //Controllo se pid non è null allora faccio il dovuto assegnamento 
    if (pid != NULL){

        //Assegna l’identificativo del processo corrente a *pid
        *pid = ACTIVE_PCB;

    }

    //Assegna l'identificativo del processo genitore a *ppid
    if (ppid != NULL){
        
        //Assegno l'id del genitore
        *ppid = ACTIVE_PCB->p_parent;
        
    }

}
