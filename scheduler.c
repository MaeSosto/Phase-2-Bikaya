#include "include/scheduler.h"
#include "include/pcb.h"
#include "include/listx.h"
#include "types_bikaya.h"

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


#define TIME_SLICE 3000

extern struct list_head* ready_queue;
struct pcb_t *ACTIVE_PCB = NULL;



void ContextSwitch();

//Setta un Time slice di 3000ms e alterna i processi in coda sulla Ready Queue e li carica nel processore
void Scheduling(){

//   //Prendo il processo in testa alla ready queue
//   ACTIVE_PCB = removeProcQ(ready_queue);

//   //Setto il timer
//   *(unsigned int*)BUS_REG_TIMER = TIME_SLICE;
  
//   //Carico il processo nel processore
//   LDST(&ACTIVE_PCB->p_s);   

	//La coda dei processi non è vuota
	if(!emptyProcQ(ready_queue)){
		
		tprint("Scheduler: coda dei processi non vuota \n");
		//Faccio un context switch per prendere il processo successivo
		ContextSwitch();
	}

	//La coda è vuota
	else{

		tprint("Scheduler: coda dei processi vuota \n");

		//Ho processi in esecuzione
		if(ACTIVE_PCB != NULL){
			
			tprint("Scheduler: cho processi in esecuzione \n");
			//Metto via il processo corrente in cpu e ne prendo un'altro
			ContextSwitch();

		}

		//Non ho processi in esecuzione
		else{

			//Controllo se ho processi bloccati
			tprint("Scheduler: non ho processi in esecuzione \n");

			HALT();
		}
	}

  
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

void ContextSwitch(){
  
  tprint("Context switch\n");

	//Ho un processo in esecuzione
	if(ACTIVE_PCB != NULL){

tprint("Context: ho processi in esecuzione \n");
		
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
    
tprint("Context : non ho processi in esecuzione \n");


  
  	}

	//Prendo il processo in testa alla ready queue
	ACTIVE_PCB = removeProcQ(ready_queue);
  
    //Setto il timer del processo
    *(unsigned int*)BUS_REG_TIMER = TIME_SLICE;
	
	//Carico il processo nel processore
   	LDST(&ACTIVE_PCB->p_s);   

}



