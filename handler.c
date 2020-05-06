
#include "include/handler.h"

void bp_handler_DOIO(){}

void bp_hadler_term(){}

void bp_sys_sot(){}

#define INTERRUPT_PENDING_MASK     	0x0000ff00
#define INTERRUPT_PENDING_B      	8
#define INTERRUPT_PENDING_FUNC(x)   (((x) & INTERRUPT_PENDING_MASK) >> INTERRUPT_PENDING_B)

#define INT_IP_GET(cause) ((cause >> 8) & 0xFF)

int tempo=FALSE;

//Gestore degli interrupt
void interruptHandler(){
	
	tempo = FALSE;

	//Se il processo non è NULL gestisco il tempo
	if(ACTIVE_PCB != NULL){
		
		//Salvo il valore del tempo in user mode perché sto entrando in kernel mode 
		ACTIVE_PCB->user_total += getTODLO() - ACTIVE_PCB->user_start;

		//Resetta il timer parziale che usiamo per tenere traccia del tempo passato in user mode
		ACTIVE_PCB->user_start = 0;
		
		//inizio a contare il tempo in kernel mode
		ACTIVE_PCB->kernel_start = getTODLO();
	}
	 
	//Prendo l' old area dell'interrupt al processo
	struct state *AREA=(state_t *) INT_OLDAREA;
	//SaveOldState(AREA, &(ACTIVE_PCB->p_s));

	//*(unsigned int*)BUS_REG_TIMER = TIME_SLICE;
	
	unsigned int cause;

  	#ifdef TARGET_UMPS

		//Accedo alla Old Area della system call
		cause = AREA->cause;
	
	#endif

	#ifdef TARGET_UARM
    
    	//Accedo alla Old Area della system call
		cause = AREA->CP15_Cause;

		//Decremento il program counter dell'interrupt old area di una word
		AREA->pc = AREA->pc - 4;

	#endif

	
	//Linee interrupt da confrontare per trovare l'interrupt giusto fra gli 8 possibili
	
	//Interrupt 1 - Inter-processor interrupts
	if(CAUSE_IP_GET(cause, INT_T_SLICE)){
		
		termprint("Interrupt: 1 \n");
	
	}
	
	//Interrupt 2 Interval Timer
	else if(CAUSE_IP_GET(cause, INT_TIMER)){

		//termprint("Interrupt: 2 \n");
		InterruptIntervalTimer();

	}
	
	//Interrut 3 - Disk
	else if(CAUSE_IP_GET(cause, INT_DISK)){
		
		termprint("Interrupt: 3 \n");
		
	}
	
	//Interrupt 4 - Tape
	else if(CAUSE_IP_GET(cause, INT_TAPE)){

		//termprint("Interrupt: 4 \n");
		InterruptTape();
	
	}
	
	//Interrupt 5 - Network
	else if(CAUSE_IP_GET(cause, INT_UNUSED)){	
			
		termprint("Interrupt: 5 \n");
	
	}
	
	//Interrupt 6 - Printer
	else if(CAUSE_IP_GET(cause, INT_PRINTER)){
		
		termprint("Interrupt: 6 \n");
		
	}

	//Interrupt 7- Terminal
	else if(CAUSE_IP_GET(cause, INT_TERMINAL)){
		
		//termprint("Interrupt: 7 \n");

		InterruptTerminal();

	}

	//NON SO
	else{
		
		termprint("Interrupt: err\n");
		
	}

	//Inviamo ACK a CP0
  	//*(unsigned int*)BUS_REG_TIMER = TIME_SLICE;

	//Non ho processi attivi in cpu
	if((tempo) | (ACTIVE_PCB==NULL)){
		
		if(ACTIVE_PCB != NULL){
			//Salvo i registri dell'old area dell'interrupt al processo
			struct state *AREA=(state_t *) INT_OLDAREA;

			/* Copio lo stato della old area dell'intertupt nel processo che lo ha sollevato */
			SaveOldState(AREA, &(ACTIVE_PCB->p_s));

			//Salvo il valore del tempo in kernel mode perchè sto entrando in user mode 
			ACTIVE_PCB->kernel_total += getTODLO() - ACTIVE_PCB->kernel_start;
			
			//Faccio partire l'user mode perchè finisco un interrupt
			ACTIVE_PCB->user_start = getTODLO();
		
		}

	
		//Richiamo lo scheduler
		Scheduling();

	}


	//Controllo se ho processi attivi
	else if(ACTIVE_PCB != NULL){
		
		// //Salvo il valore del tempo in kernelmode perchè sto entrando in user mode 
		// ACTIVE_PCB->kernel_total += getTODLO() - ACTIVE_PCB->kernel_start;

		Scheduling();
		
	}

}

//====================================================================================================================

//Gestore delle system call
void syscallHandler(){

	//Se il processo non è NULL gestisco il tempo
	if(ACTIVE_PCB != NULL){
		
		//Salvo il valore del tempo in user mode perché sto entrando in kernel mode 
		ACTIVE_PCB->user_total += getTODLO() - ACTIVE_PCB->user_start;

		//Resetta il timer parziale che usiamo per tenere traccia del tempo passato in user mode
		ACTIVE_PCB->user_start = 0;
		
		//inizio a contare il tempo in kernel mode
		ACTIVE_PCB->kernel_start = getTODLO();

	}

	unsigned int cause;
	
	unsigned int ritorno; //Assegno il valore di ritorno

	struct state *AREA = (state_t *) SYSBK_OLDAREA;
	
	SaveOldState((state_t *) SYSBK_OLDAREA, &(ACTIVE_PCB->p_s));


	//cpy_state((state_t*) SYSBK_OLDAREA, &curr_proc->p_s);
	
  	#ifdef TARGET_UMPS
		
		//Aumentiamo di una word
		AREA->pc_epc = AREA->pc_epc + 4;
		
		//Accedo alla Old Area della system call
		cause = (CAUSE_GET_EXCCODE(AREA->cause));
	
	#endif

	#ifdef TARGET_UARM
    
    	//Accedo alla Old Area della system call
		cause = CAUSE_EXCCODE_GET(AREA->CP15_Cause);
        
	#endif

    //SYSCALL
    if(cause == EXC_SYSCALL){

		//termprint("SYS \n");

		//SYSCALL 1
		if(AREA->reg_a0 == GETCPUTIME){
			
			//termprint("SYS 1 \n");
			getCPUTime(&AREA->reg_a1, &AREA->reg_a2, &AREA->reg_a3);

		}

		//SYSCALL 2
		else if(AREA->reg_a0 == CREATEPROCESS){
			
			//termprint("SYS 2 \n");
			ritorno =  CreateProcess((state_t*)AREA->reg_a1, (int)AREA->reg_a2, (void **)AREA->reg_a3);			
			

		}

		//SYSCALL 3
		else if(AREA->reg_a0 == TERMINATEPROCESS){
			
			termprint("SYS 3 \n");
			ritorno = TerminateProcess((void **)AREA->reg_a1);

		}

		//SYSCALL 4
		else if(AREA->reg_a0 == VERHOGEN){

			//termprint("SYS 4 \n");
			//insert = TRUE;
			Verhogen((int*)AREA->reg_a1);
			
		}

		//SYSCALL 5
		else if(AREA->reg_a0 == PASSEREN){
			
			//termprint("SYS 5 \n");
			//insert = TRUE;
			Passeren((int*)AREA->reg_a1);

		}

		//SYSCALL 6
		else if(AREA->reg_a0 == WAITIO){

			GOODMORNING_PCB=ACTIVE_PCB;	
			//termprint("SYS 6 \n");
			ritorno = DO_IO((unsigned int)AREA->reg_a1, (unsigned int*)AREA->reg_a2, (int)AREA->reg_a3);	
			bp_handler_DOIO();
		

		}

		//SYSCALL 7
		else if(AREA->reg_a0 == SPECPASSUP){
			
			termprint("SYS 7 \n");
			ritorno = SpecPassup(AREA->reg_a1, (struct state*)AREA->reg_a2, (struct state*)AREA->reg_a3);
			
		}

		//SYSCALL 8
		else if(AREA->reg_a0 == GETPID){
			
      		termprint("SYS 8 \n");
			getPid((void **)AREA->reg_a1, (void **)AREA->reg_a2);
		
		}

		else{

			termprint("SYS ALTRA \n");
			//Inoltro al gestore di livello superiore (Spec_Passup) oppure termina
		}

    }

	// //Controllo che sia un Breakpoint (EXC_BP 9)
    // else if(CAUSE_GET_EXCCODE(AREA->cause) == EXC_BP){
    
    // 	termprint("E' partito un Breakpoint \n");
    
    // }

	//Ho un processo ancora attivo in cpu
	if(ACTIVE_PCB != NULL){

		//Salvo lo stato
		SaveOldState(AREA, &(ACTIVE_PCB->p_s));

		#ifdef TARGET_UMPS
		
			ACTIVE_PCB->p_s.reg_v0 = ritorno;
		
		#endif

		#ifdef TARGET_UARM

			ACTIVE_PCB->p_s.reg_a0 = ritorno;
		
		#endif

		//Salvo il valore del tempo in kernel mode perchè sto entrando in user mode 
		ACTIVE_PCB->kernel_total += getTODLO() - ACTIVE_PCB->kernel_start;
			
		//Faccio partire l'user mode perchè finisco un interrupt
		ACTIVE_PCB->user_start = getTODLO();

		LDST(&ACTIVE_PCB->p_s);

	}

	//Non ho più processi attivi sulla cpu
	else{

		//termprint("Sys: vado nello scheduler \n");
		
		//Chiamo lo scheduler
		Scheduling();
	
	}

}

//Gestore delle trap
void trapHandler(){

  //Non faccio niente per ora
  #ifdef TARGET_UMPS
  
    termprint("Sto gestendo una trap \n");
  
  #endif

  #ifdef TARGET_UARM
  
    tprint("Sto gestendo una trap \n");
  
  #endif

}

//Gestore delle tlb
void tlbHandler(){

  //Non faccio niente per ora  
  #ifdef TARGET_UMPS
  
    termprint("Sto gestendo la TLB \n");
  
  #endif
  
  #ifdef TARGET_UARM
  
    tprint("Sto gestendo la TLB \n");
  
  #endif

}

