#include "include/types_bikaya.h"
#include "include/listx.h"
#include "include/pcb.h"
#include "include/asl.h"
#include "include/utils.h"
#include "include/const_bikaya.h"


#ifdef TARGET_UMPS

	#include <umps/libumps.h> 
    #include <umps/types.h>
    #include <umps/arch.h>
    #define INT_OLDAREA 0x20000000
    #define SYSCALL_OLDAREA 0x20000348
    	#define MAX_DEVICES (DEV_USED_INTS * DEV_PER_INT) + DEV_PER_INT + 1

#endif

#ifdef TARGET_UARM

    #include <uarm/arch.h>
    #include <uarm/uARMtypes.h>
    #include <uarm/uARMconst.h>

#endif

extern struct pcb_t *ACTIVE_PCB;
extern struct list_head* ready_queue;
extern struct device_semd Semaforo;
extern int SemMem[MAX_DEVICES];
extern void termprint(char *str);
extern int insert;

#define BUS_TODLOW  0x1000001c
#define BUS_TODHIGH 0x10000018
#define getTODLO() (*((unsigned int *)BUS_TODLOW))


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

	// Controlo se ho 1 o più thread nella lista d'attesa
	if (*semaddr <= 0){
       
        //Prendo il primo processo messo in attesa
	 	pcb_t* pcb_blocked = removeBlocked(semaddr);

        //Rimetto la priorità originale del semaforo
	 	pcb_blocked->priority = pcb_blocked->original_priority;

        //Inserisco il processo nella ready queue
        insertProcQ(ready_queue, pcb_blocked);
        
    }

    if(insert){

        //Salvo il processo corrente e lo rimetto nella ready queue
	    SaveProc();
        termprint("Salvo nella verhogen \n");

    }

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
        
        //Metto il processo nella coda del semaforo
	 	int ret = insertBlocked(semaddr, ACTIVE_PCB);
        
        //assegnamento al semd NON andato a buon fine
        if(ret){
            PANIC();
        }
     
    }

    if(insert){

        //Salvo il processo corrente e lo rimetto nella ready queue
	    SaveProc();
        termprint("Salvo nella passaren \n");

    }
    
    // L'ACTIVE PCB VA MESSO A NULL ALLA FINE DELLA SYSCALL PRIMA DI CHIAMARE LO SCHEDULER

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
    
    int dev = 0;
    int line = 0;
    int status = 0;

    dev = numDev(registro);
    line = numLine(registro);

    //Blocco il processo 
    if(line == 3)         Passeren(Semaforo.disk[dev].s_key);
    else if(line == 4)    Passeren(Semaforo.tape[dev].s_key);
    else if(line == 5)    Passeren(Semaforo.network[dev].s_key);
    else if(line == 6)    Passeren(Semaforo.printer[dev].s_key);
    else if(line == 7)    Passeren(Semaforo.terminal[dev].s_key);
    
    // termprint("La linea e': ");
    // stampaInt(line);
    // termprint("\nIl device e': ");
    // stampaInt(dev);
    // termprint("\n");

    //Non è un terminale
    if(line < 7){
        
        dtpreg_t *devreg = (dtpreg_t *) registro;
        devreg->command = command;
        status = devreg->status;
        termprint ("Setto lo status 1 \n");

    }

    //E' un terminale
    else{

        termreg_t *termreg = (termreg_t*) registro;

        //Trasmissione
        if(subdevice == FALSE){

            status = termreg->transm_status;
            termprint ("Setto lo status 2 \n");
            termreg->transm_command = command;


        }
        //Ricezione
        else{
            
            status = termreg->recv_status;
            termprint ("Setto lo status 3 \n");
            termreg->recv_command = command;
           
            
        }
       
    }


    // //il processo che richiede I/O va bloccato
	// if(wakeup_proc == curr_proc){
	// 		Passeren(semaddr);
	// }
	// else{
	// 	/*se il processo ad aver chiamato la Do_IO è il processo risvegliato dopo un'operazione di I/O
	// 	allora non si richiama lo scheduler dopo aver fatto la P perciò non utilizzo la Passeren*/
	// 	(*semaddr)--;
	// 	ProcBlocked++;
	// 	insertBlocked(semaddr, wakeup_proc);
	// 	outProcQ(&ready_queue_h, wakeup_proc);
	// }


    insert = TRUE;

    if(insert){

        //Salvo il processo corrente e lo rimetto nella ready queue
	    SaveProc();
        termprint("Salvo nella do_io \n");

    }

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