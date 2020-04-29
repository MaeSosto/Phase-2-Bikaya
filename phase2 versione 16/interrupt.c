#include "include/interrupt.h"

//Gestore degli interrupt
void interruptHandler(){
	
	termprint("Interrupt \n");
	// salvo il valore del tempo in kernelmode perchè sto entrando in user mode 
	//ACTIVE_PCB->user_total += getTODLO() - ACTIVE_PCB->user_start;
	//inizio a contare il tempo in user mode
	//ACTIVE_PCB->kernel_start = getTODLO();


	AREA=(state_t *) INT_OLDAREA;
	unsigned int cause;
	//Salvo i registri dell'old area dell'interrupt al processo 
    state_t* oldarea = ((state_t*)INT_OLDAREA);

    
  	#ifdef TARGET_UMPS

		//Accedo alla Old Area della system call
		cause = (CAUSE_GET_EXCCODE(AREA->cause));
	
	#endif

	#ifdef TARGET_UARM
    
    	//Accedo alla Old Area della system call
		cause = CAUSE_EXCCODE_GET(AREA->CP15_Cause);

		//Decremento il program counter dell'interrupt old area di una word
		oldarea->pc = oldarea->pc - 4;

	#endif

    //linee interrupt da confrontare per trovare l'interrupt giusto fra gli 8 possibili
    if(cause == INT_T_SLICE){
        
    }

	

	//Inviamo ACK a CP0
  	*(unsigned int*)BUS_REG_TIMER = TIME_SLICE;


	//Ho già un processo attivo
	if(ACTIVE_PCB != NULL){

		//Tornare in user mode

		/* Copio lo stato della old area dell'intertupt nel processo che lo ha sollevato */
		SaveOldState(oldarea, &(ACTIVE_PCB->p_s));
		
	}

	//Non ho processi in esecuzione
	else{

		//Riparto con lo scheduler

	}

	//Richiamo lo scheduler
 	Scheduling();

}