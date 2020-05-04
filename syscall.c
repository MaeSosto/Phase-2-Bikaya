#include "include/syscall.h"

#define TERMSTATMASK 0xFF





// void bp_status1(){}
// void bp_status2(){}
// void bp_status3(){}
// void bp_status4(){}
// void bp_status5(){}

bp_passaren_end(){}

//SYSCALL 1
void getCPUTime(unsigned int *user, unsigned int *kernel, unsigned int *wallclock){
    
    // void SYSCALL(GETCPUTIME, unsigned int *user, unsigned int *kernel, unsigned int *wallclock)
    // Quando invocata, la SYS1 restituisce il tempo di
    // esecuzione del processo che l’ha chiamata fino a
    // quel momento, separato in tre variabili:
    // –Il tempo usato dal processo come utente (user)
    // –Il tempo usato dal processo come kernel (tempi
    // di system call e interrupt relativi al processo)
    // –Tempo totale trascorso dalla prima attivazione
    // del processo.
    //

    
     ACTIVE_PCB->kernel_total = ACTIVE_PCB->kernel_total + getTODLO()-ACTIVE_PCB->kernel_start;
     ACTIVE_PCB->user_total = ACTIVE_PCB->user_total + getTODLO()-ACTIVE_PCB->user_start;

     *(user) = ACTIVE_PCB->user_total;
     *(kernel) =ACTIVE_PCB ->kernel_total;
     *(wallclock) =getTODLO()-ACTIVE_PCB->wallclock_start;

}

//SYSCALL 2
int CreateProcess(struct state *statep, int priority, void ** cpid){

    // int SYSCALL(CREATEPROCESS, state_t *statep, int priority, void ** cpid)
    // – Questa system call crea un nuovo processo
    // come figlio del chiamante. Il program counter, lo
    // stack pointer, e lo stato sono indicati nello stato
    // iniziale. Se la system call ha successo il valore di
    // ritorno è 0 altrimenti è -1. Se cpid != NULL e la
    // chiamata ha successo *cpid contiene
    // l’identificatore del processo figlio, rappresentato
    // dall’indirizzo del suo pcb_t.

    //Creo nuovo processo figlio
    pcb_t* tempPcb = allocPcb();
    
    // Controllo che l'indirizzo sia corretto
    *((pcb_t **)cpid) = tempPcb;

    if ((cpid != NULL) && (tempPcb)){ //Ha successo: cpid non NULL e tempPcb allocato correttamente

        //Assegno lo stato del nuovo processo figlio
        #ifdef TARGET_UMPS  

            tempPcb->p_s.status = statep->status;
            tempPcb->p_s.reg_sp = statep->reg_sp;
            tempPcb->p_s.pc_epc = statep->pc_epc;     

        #endif

        #ifdef TARGET_UARM

            tempPcb->p_s.cpsr = statep->cpsr; 
            tempPcb->p_s.CP15_Control = statep->CP15_Control;
            tempPcb->p_s.sp = statep->sp;
            tempPcb->p_s.pc = statep->pc;

        #endif

        //SETTA IL TEMPO

        tempPcb->priority = tempPcb->original_priority = priority;

        //Inserisco tempPcb come figlio di ACTIVE_PCB
        insertChild(ACTIVE_PCB, tempPcb);

        //Inserisco il figlio nella ready queue
        insertProcQ(ready_queue, tempPcb);

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

    // void SYSCALL(VERHOGEN, int *semaddr, 0, 0)
    // – Operazione di rilascio su un semaforo. Il valore
    // del semaforo è memorizzato nella variabile di
    // tipo intero passata per indirizzo. L’indirizzo
    // della variabile agisce da identificatore per il
    // semaforo.
    
    //  Il semaforo viene incrementato. Se ci sono task in coda, uno dei task in coda il primo viene tolto dalla coda, posto in stato di ready (sarà perciò eseguito appena schedulato dal sistema operativo).
    // V(): incrementa il valore. Nel caso in cui ho 1 o più thread nella lista d'attesa, prelevo il primo e lo inserisco nella lista dei pronti. Se non sono presenti thread nella lista d'attesa il risultato sarà che avremo la variabile incrementata di 1.

    //Incremento il semaforo
    *semaddr+=1;

	// Controllo se ho più thread nella lista d'attesa
	if (*semaddr <= 0){      
        
        //Prendo il primo processo messo in attesa
	    pcb_t* pcb_blocked = removeBlocked(semaddr);

        //Aggiorno il contatore dei processi bloccati
        BLOCK_COUNT--;
        
        
        //Mi salvo il processo che ho appena svegliato per poi aggiornare il suo status quando lo sbloccherò
        GOODMORNING_PCB = pcb_blocked;

        //Rimetto la priorità originale del semaforo
	 	//pcb_blocked->priority = pcb_blocked->original_priority;

        //Salvo i registri dell'old area della sys al processo 
        //state_t* oldarea = ((state_t*)SYSCALL_OLDAREA);

        //Copio lo stato della old area della sys nel processo che lo ha sollevato 
        //SaveOldState(oldarea, &(ACTIVE_PCB->p_s));

        //Inserisco il processo nella ready queue
        insertProcQ(ready_queue, pcb_blocked);
        
    }

    // if(insert){

    //     //Salvo il processo corrente e lo rimetto nella ready queue
    //	   SaveProc();
    //     termprint("Salvo nella verhogen \n");

    // }

}

//SYSCALL 5 - metto processo in attesa su un s
void Passeren(int *semaddr){

    // void SYSCALL(PASSEREN, int *semaddr, 0, 0)
    // – Operazione di richiesta di un semaforo. Il valore
    // del semaforo è memorizzato nella variabile di
    // tipo intero passata per indirizzo. L’indirizzo
    // della variabile agisce da identificatore per il
    // semaforo.
    
    //  Il semaforo viene decrementato. Se, dopo il decremento, il semaforo ha un valore negativo, il task viene sospeso e accodato, in attesa di essere riattivato da un altro task.

    //Decremento il semaforo
    *semaddr-=1;

    //Controllo se ci sono altri processi bloccati
    if (*semaddr < 0){
        
        //Salvo i registri dell'old area della sys al processo 
        state_t* oldarea = ((state_t*)SYSCALL_OLDAREA);

        //Copio lo stato della old area della sys nel processo che lo ha sollevato 
        SaveOldState(oldarea, &(ACTIVE_PCB->p_s));

        //Metto il processo nella coda del semaforo
	 	int ret = insertBlocked(semaddr, ACTIVE_PCB);
        
        //Aggiorno il contatore dei processi bloccati
        BLOCK_COUNT ++;
        
        // L'ACTIVE PCB VA MESSO A NULL ALLA FINE DELLA SYSCALL PRIMA DI CHIAMARE LO SCHEDULER
        ACTIVE_PCB = NULL;

        bp_passaren_end();

        //assegnamento al semd NON andato a buon fine
        if(ret){

            PANIC();
        
        }
     
    }    

}

//SYSCALL 6
int DO_IO(unsigned int command, unsigned int* registro, int subdevice){

    // // int SYSCALL(IOCOMMAND, unsigned int command, unsigned int *register, int subdevice)
    // // – Questa system call attiva una operazione di I/O
    // // copiando parametro command nel campo
    // // comando del registro del dispositivo indicato
    // // come puntatore nel secondo argomento.
    // // –L’operazione è bloccante, quindi il chiamante
    // // viene sospeso sino alla conclusione del comando.
    // // Il valore ritornato è il contenuto del registro di
    // // status del dispositivo.
    // // –Il quarto parametro indica a quale sottodevice si
    // // sta facendo riferimento nel caso in cui si voglia
    // // portare avanti un’operazione su un terminale. 0
    // // corrisponde alla trasmissione, 1 alla ricezione.

    //if(ACTIVE_PCB != NULL){
        
       // ACTIVE_PCB->command=command;

    //}
    
    
    int *sem;
    int dev = 0;
    int line = 0;
    int status = 0;

    dev = numDev((unsigned int *)registro);
    line = numLine((unsigned int *)registro);

    // termprint("La linea e': ");
    // stampaInt(line);
    // termprint("\nIl device e': ");
    // stampaInt(dev);
    // termprint("\n");

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
            if(!*sem){ //Entra qua se il contenuto di sem è 0, ovvero se non ci sono processi bloccati su quel semaforo

                termreg->transm_command = command;
            
            }
         

            status = termreg->transm_status;

        }

        //Ricezione
        else{


            /*dai nostri amici trattino         
            - richiesta di I/O su un terminale in trasmissione, si scrive il comando nel campo transm_command
            - semaforo con valore 1, terminale libero in trasmissione
            */
            //Blocco il processo 
            sem = Semaforo.terminalR[dev].s_key;
            
            if(!*sem){ //Entra qua se il contenuto di sem è 0, ovvero se non ci sono processi bloccati su quel semaforo

                termreg->recv_command = command;
         
            }
     

            status = termreg->recv_status;
                                   
        }
       
    }

    //Qua sto venendo da una sys
    if(GOODMORNING_PCB == ACTIVE_PCB){
        
        Passeren(sem);
        //Dopo la passeren l'active pcb è a null, gli riassegno il processo svegliato
        //ACTIVE_PCB = GOODMORNING_PCB;

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