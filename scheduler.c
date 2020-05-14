#include "include/scheduler.h"

void bp_aging(){}


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

	//Ho un processo in esecuzione
	if(ACTIVE_PCB != NULL){
		
		//Ripristiniamo l'original_priority del processo appena concluso
  		ACTIVE_PCB->priority = ACTIVE_PCB->original_priority;

		//Faccio l'aging
		Aging();
		
		//Il processo smette di essere quello attivo quindi termina la sua user mode
		stopUserTime(ACTIVE_PCB);

		//Metto il processo nella Ready Queue
  		insertProcQ(ready_queue, ACTIVE_PCB);

		ACTIVE_PCB = NULL;

	}
	
	//Prendo il processo in testa alla ready queue
	ACTIVE_PCB = removeProcQ(ready_queue);

	//Se non ho ancora settato il tempo iniziale
	if(!getWallclockTime(ACTIVE_PCB)){q

		//Assegno il tempo iniziale
		setWallclockTime(ACTIVE_PCB);

	}

	//Inizio a contare il tempo in user mode
	startUserTime(ACTIVE_PCB);

    //Setto il timer di 3ms che gestisce lo switch tra i processi
    *(unsigned int*)BUS_REG_TIMER = TIME_SLICE;
	
	//Carico il processo nel processore
   	LDST(&ACTIVE_PCB->p_s);   

}

//Setta un Time slice di 3000ms e alterna i processi in coda sulla Ready Queue e li carica nel processore
void Scheduling(){

	//La coda dei processi non è vuota
	if(!emptyProcQ(ready_queue)){
		
		//Faccio un context switch per prendere il processo successivo
		ContextSwitch();
	}

	//La coda è vuota
	else{

		//termprint("Scheduler: coda dei processi vuota \n");

		//Ho processi in esecuzione
		if(ACTIVE_PCB != NULL){

			//Metto via il processo corrente in cpu e ne prendo un'altro
			ContextSwitch();

		}

		//Non ho processi in esecuzione
		else{

			//Controllo se ho processi bloccati nei semafori
			if(BLOCK_COUNT > 0){

				//Ho solo processi bloccati, aspetto

				//Setto il timer
  				*(unsigned int*)BUS_REG_TIMER = TIME_SLICE;

				
				#ifdef TARGET_UMPS
	
					//Abilito tutti gli interrupt e vado in kernel mode
					// setSTATUS(getSTATUS() | STATUS_IEc | STATUS_IM_MASK);
					setSTATUS((getSTATUS() | STATUS_IEc) | STATUS_IM_MASK);//interrupt abilitati
				
				#endif
				
				#ifdef TARGET_UARM

					//Abilito tutti gli interrupt e vado in kernel mode
					setSTATUS(getSTATUS() | STATUS_SYS_MODE);
					setSTATUS(STATUS_ALL_INT_ENABLE(getSTATUS()));
					setSTATUS(STATUS_ENABLE_TIMER(getSTATUS()));
					setSTATUS(STATUS_ENABLE_INT(getSTATUS()));

				#endif

				//aspetto che si sollevi un interrupt da un device. Questo mi sblocca un processo in attesa sul semaforo del device.
				WAIT();
			}
			
			//Non ci sono processi in ready queue, nè attivi, nè bloccati sui semafori
			else{

				HALT();

			}
		}
	}
}
