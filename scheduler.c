#include "include/scheduler.h"

void bp_aging(){}


//Auementa di 1 unità la priorità di ogni processo in coda e reimposta la priorità origiale del processo che è appena stato eseguito e lo rimette in attesa
void Aging(){
   
	struct list_head *tempList = NULL;
	struct pcb_t *tempPcb = NULL;

	//Aumentiamo di 1 tutte le priorità dei processi che non sono in esecuzione
	list_for_each(tempList, ready_queue){
		bp_aging();
		tempPcb = container_of(tempList, struct pcb_t, p_next);
		tempPcb->priority = tempPcb->priority + 1;

	}

}

void ContextSwitch(){
  
  	//termprint(" Entro nel context switch\n");

	//Ho un processo in esecuzione
	if(ACTIVE_PCB != NULL){

		//termprint("Context: ho processi in esecuzione \n");
		
		//Ripristiniamo l'original_priority del processo appena concluso
  		ACTIVE_PCB->priority = ACTIVE_PCB->original_priority;

		//Faccio l'aging
		Aging();

		//Metto il processo nella Ready Queue
  		insertProcQ(ready_queue, ACTIVE_PCB);

		ACTIVE_PCB = NULL;
	}
	
	//Non ho processi in esecuzione
	else{
		
		//termprint("Context : non ho processi in esecuzione \n");


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
	
	//termprint("Scheduler: HO TOT PROCESSI BLOCCATI: ");
	//stampaInt(BLOCK_COUNT);


	// // salvo il valore del tempo in kernelmode perchè sto entrando in user mode 
	// ACTIVE_PCB->kernel_total += getTODLO() - ACTIVE_PCB->kernel_start;
	// //inizio a contare il tempo in user mode
	// ACTIVE_PCB->user_start = getTODLO();
	
	//La coda dei processi non è vuota
	if(!emptyProcQ(ready_queue)){
		
		//termprint("Scheduler: coda dei processi non vuota \n");
		//Faccio un context switch per prendere il processo successivo
		ContextSwitch();
	}

	//La coda è vuota
	else{

		//termprint("Scheduler: coda dei processi vuota \n");

		//Ho processi in esecuzione
		if(ACTIVE_PCB != NULL){
			
			//termprint("Scheduler: cho processi in esecuzione \n");
			//Metto via il processo corrente in cpu e ne prendo un'altro
			ContextSwitch();

		}

		//Non ho processi in esecuzione
		else{

			//Controllo se ho processi bloccati nei semafori
			//termprint("Scheduler: non ho processi in esecuzione \n");

			//termprint("Scheduler: Ho ");
			//stampaInt(BLOCK_COUNT);
			//termprint("processi bloccati \n");

			//Controllo se ho processi bloccati
			if(BLOCK_COUNT > 0){

				//termprint("Scheduler: Ho solo processi bloccati, aspetto \n");

				//Setto il timer
  				*(unsigned int*)BUS_REG_TIMER = TIME_SLICE;

				
				#ifdef TARGET_UMPS
	
					//Abilito tutti gli interrupt e vado in kernel mode
					// setSTATUS(getSTATUS() | STATUS_IEc | STATUS_IM_MASK);
						setSTATUS((getSTATUS() | STATUS_IEc) | STATUS_IM_MASK);//interrupt abilitati
				
				#endif
				
				#ifdef TARGET_UARM

					//tprint("VEDI UN PO CHE FARE QUA \n");
					//per uarm non setto lo status per now

				#endif
				
				//aspetto che si sollevi un interrupt da un device. Questo mi sblocca un processo in attesa sul semaforo del device.
				WAIT();
			}
			
			//Non ci sono processi in ready queue, nè attivi, nè bloccati sui semafori
			else{
				
				//termprint("Scheduling: Non ci sono processi in ready queue, ne' attivi, ne' bloccati sui semafori \n");
				HALT();

			}
		}
	}
}
