#include "include/scheduler.h"
#include "include/pcb.h"
#include "include/listx.h"
#include "types_bikaya.h"
#include "include/utils.h"

#ifdef TARGET_UMPS

	#include "umps/libumps.h"
	#include "umps/arch.h"
	extern void termprint(char* str);

#endif

#ifdef  TARGET_UARM

	#include <uarm/arch.h>
	#include <uarm/libuarm.h>
	extern void tprint(char* str);
	
#endif

#define BUS_TODLOW  0x1000001c
#define BUS_TODHIGH 0x10000018
#define getTODLO() (*((unsigned int *)BUS_TODLOW))
#define TIME_SLICE 3000

extern struct list_head* ready_queue;
struct pcb_t *ACTIVE_PCB = NULL;



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

void ContextSwitch(){
  
  	//termprint(" Entro nel context switch\n");

	//Ho un processo in esecuzione
	if(ACTIVE_PCB != NULL){

		termprint("Context: ho processi in esecuzione \n");
		
		//Ripristiniamo l'original_priority del processo appena concluso
  		ACTIVE_PCB->priority = ACTIVE_PCB->original_priority;

		//Faccio l'aging
		Aging();

		//Rimetto il processo in attesa nella Ready Queue
  		insertProcQ(ready_queue, ACTIVE_PCB);

		ACTIVE_PCB = NULL;
	}
	
	//Non ho processi in esecuzione
	else{
    
		termprint("Context : non ho processi in esecuzione \n");


  	}

	//Prendo il processo in testa alla ready queue
	ACTIVE_PCB = removeProcQ(ready_queue);
  
    //Setto il timer del processo
    *(unsigned int*)BUS_REG_TIMER = TIME_SLICE;
	
	//Carico il processo nel processore
   	LDST(&ACTIVE_PCB->p_s);   

}

//Setta un Time slice di 3000ms e alterna i processi in coda sulla Ready Queue e li carica nel processore
void Scheduling(){

	// // salvo il valore del tempo in kernelmode perchè sto entrando in user mode 
	// ACTIVE_PCB->kernel_total += getTODLO() - ACTIVE_PCB->kernel_start;
	// //inizio a contare il tempo in user mode
	// ACTIVE_PCB->user_start = getTODLO();
	
	//La coda dei processi non è vuota
	if(!emptyProcQ(ready_queue)){
		
		termprint("Scheduler: coda dei processi non vuota \n");
		//Faccio un context switch per prendere il processo successivo
		ContextSwitch();
	}

	//La coda è vuota
	else{

		termprint("Scheduler: coda dei processi vuota \n");

		//Ho processi in esecuzione
		if(ACTIVE_PCB != NULL){
			
			termprint("Scheduler: cho processi in esecuzione \n");
			//Metto via il processo corrente in cpu e ne prendo un'altro
			ContextSwitch();

		}

		//Non ho processi in esecuzione
		else{

			//Controllo se ho processi bloccati
			termprint("Scheduler: non ho processi in esecuzione \n");

			HALT();
		}
	}
}
