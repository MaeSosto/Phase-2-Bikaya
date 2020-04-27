#include "include/scheduler.h"
#include "include/pcb.h"
#include "include/listx.h"
#include "types_bikaya.h"
#include "include/utils.h"

#ifdef TARGET_UMPS

  #include "umps/libumps.h"
  #include "umps/arch.h"

#endif

#ifdef  TARGET_UARM

  #include <uarm/arch.h>
  #include <uarm/libuarm.h>

#endif

#define BUS_TODLOW  0x1000001c
#define BUS_TODHIGH 0x10000018
#define getTODLO() (*((unsigned int *)BUS_TODLOW))
#define TIME_SLICE 3000

extern struct list_head* ready_queue;
struct pcb_t *ACTIVE_PCB = NULL;
extern void termprint(char *str);

//Setta un Time slice di 3000ms e alterna i processi in coda sulla Ready Queue e li carica nel processore
void Scheduling(){

	//Se ci sono processi sulla ready queue allora li considero altrimenti halt
	if(emptyProcQ(ready_queue)){
		
		termprint("Coda vuota \n");
		HALT();

	}

	// termprint ("Scheduler: riprendo il processo \n");

	//Prendo il processo in testa alla ready queue
	ACTIVE_PCB = removeProcQ(ready_queue);
	
	termprint("\n Esco il processo: ");
	stampaInt(ACTIVE_PCB->original_priority);
	termprint("\n");
	
	// salvo il valore del tempo in kernelmode perchè sto entrando in user mode 
	ACTIVE_PCB->kernel_total += getTODLO() - ACTIVE_PCB->kernel_start;
	//inizio a contare il tempo in user mode
	ACTIVE_PCB->user_start = getTODLO();

	//Setto il timer
	*(unsigned int*)BUS_REG_TIMER = TIME_SLICE;

	//Carico il processo nel processore
	LDST(&ACTIVE_PCB->p_s);   

}

//Auementa di 1 unità la priorità di ogni processo in coda e reimposta la priorità origiale del processo che è appena stato eseguito e lo rimette in attesa
void Aging(){
   
	struct list_head *tempList = NULL;
	struct pcb_t *tempPcb = NULL;

	//Aumentiamo di 1 tutte le priorità dei processi che non sono in esecuzione
	list_for_each(tempList, ready_queue){

	tempPcb = container_of(tempList, struct pcb_t, p_next);
	tempPcb->priority = tempPcb->priority + 1;

	}

}



