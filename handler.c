
#include "include/handler.h"

extern void stampaCauseExc(int n);

int tempo=FALSE;

//Gestore degli interrupt
void interruptHandler(){
	
	tempo = FALSE;

	//Se il processo non è NULL gestisco il tempo
	if(tempo || ACTIVE_PCB != NULL){
		
		#ifdef TARGET_UMPS
			if(ACTIVE_PCB->user_start > 0){

				//Salvo il valore del tempo in user mode perché sto entrando in kernel mode 
				ACTIVE_PCB->user_total += (getTODLO() - ACTIVE_PCB->user_start);

				//Resetta il timer parziale che usiamo per tenere traccia del tempo passato in user mode
				ACTIVE_PCB->user_start = 0;
				
			}
		
		#endif

		//inizio a contare il tempo in kernel mode
		//ACTIVE_PCB->kernel_start = getTODLO();
	}


	//Prendo l' old area dell'interrupt al processo
	state_t *AREA=(state_t *) INT_OLDAREA;

	//SaveOldAreaToPCB(AREA, &(ACTIVE_PCB->p_s));

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
	if(ACTIVE_PCB != NULL){

		#ifdef TARGET_UMPS

			//inizio a contare il tempo in kernel mode
			ACTIVE_PCB->kernel_start = getTODLO();

		#endif
	}

	#ifdef TARGET_UMPS
		//Interrupt 1 - Inter-processor interrupts
		if(CAUSE_IP_GET(cause, INT_T_SLICE)){
			
			InterruptPLC();
			//????
		// 	 setTIMER(TIME_SLICE);
		//   scheduler();
		
		}

	#endif

	#ifdef TARGET_UARM

		//Interrupt 1 - Inter-processor interrupts 
		//Non c'è un uARM
		if(0){}

	#endif
	
	//Interrupt 2 Interval Timer
	else if(CAUSE_IP_GET(cause, INT_TIMER)){

		//termprint("Interrupt: 2 \n");
		InterruptIntervalTimer();

	}
	
	//Interrut 3 - Disk
	else if(CAUSE_IP_GET(cause, INT_DISK)){
		
		InterruptDisk();
		
	}
	
	//Interrupt 4 - Tape
	else if(CAUSE_IP_GET(cause, INT_TAPE)){

		InterruptTape();
	
	}
	
	//Interrupt 5 - Network
	else if(CAUSE_IP_GET(cause, INT_UNUSED)){	
			
		InterruptNetwork();
	
	}
	
	//Interrupt 6 - Printer
	else if(CAUSE_IP_GET(cause, INT_PRINTER)){
		
		InterruptPrinter();
		
	}

	//Interrupt 7- Terminal
	else if(CAUSE_IP_GET(cause, INT_TERMINAL)){
		
		//termprint("Interrupt: 7 \n");

		InterruptTerminal();

	}

	//NON SO
	else{
		
		//termprint("Interrupt: err\n");
		PANIC();

	}

	//Inviamo ACK a CP0
  	//*(unsigned int*)BUS_REG_TIMER = TIME_SLICE;

	//Non ho processi attivi in cpu
	if((tempo) | (ACTIVE_PCB==NULL)){
		
		if(ACTIVE_PCB != NULL){
			//Salvo i registri dell'old area dell'interrupt al processo
			state_t *AREA=(state_t *) INT_OLDAREA;

			/* Copio lo stato della old area dell'intertupt nel processo che lo ha sollevato */
			SavePCBToOldArea(AREA, &(ACTIVE_PCB->p_s));

			
			//Salvo il valore del tempo in kernel mode perchè sto entrando in user mode 
			//ACTIVE_PCB->kernel_total += (getTODLO() - ACTIVE_PCB->kernel_start);
			
			#ifdef TARGET_UMPS
			
				//Faccio partire l'user mode perchè finisco un interrupt
				ACTIVE_PCB->user_start = getTODLO(); 

			#endif 
		
		}

	
		//Richiamo lo scheduler
		Scheduling();

	}


	//Controllo se ho processi attivi
	else if(ACTIVE_PCB != NULL){
		
		#ifdef TARGET_UMPS

			// //Salvo il valore del tempo in kernelmode perchè sto entrando in user mode 
			 //ACTIVE_PCB->kernel_total += getTODLO() - ACTIVE_PCB->kernel_start;

			ACTIVE_PCB->user_start = getTODLO(); 

		#endif 


		Scheduling();
		
	}

}

//====================================================================================================================

//Gestore delle system call
void syscallHandler(){

	//Se il processo non è NULL gestisco il tempo
	if(ACTIVE_PCB != NULL){
		
		#ifdef TARGET_UMPS
			
			if(ACTIVE_PCB->user_start > 0){

				//Salvo il valore del tempo in user mode perché sto entrando in kernel mode 
				ACTIVE_PCB->user_total += (getTODLO() - ACTIVE_PCB->user_start);

				//Resetta il timer parziale che usiamo per tenere traccia del tempo passato in user mode
				ACTIVE_PCB->user_start = 0;
				
			}
		
		#endif

	}

	unsigned int cause;
	
	unsigned int ritorno; //Assegno il valore di ritorno

	state_t *AREA = (state_t *) SYSBK_OLDAREA;
	
	SavePCBToOldArea((state_t *) SYSBK_OLDAREA, &(ACTIVE_PCB->p_s));


	//cpy_state((state_t*) SYSBK_OLDAREA, &curr_proc->p_s);
	
  	#ifdef TARGET_UMPS
		
		//Aumentiamo di una word
		AREA->pc_epc = AREA->pc_epc + 4;
		
		//Accedo alla Old Area della system call
		cause = (CAUSE_GET_EXCCODE(AREA->cause));

		unsigned int param0 = AREA->reg_a0;
		unsigned int param1 = AREA->reg_a1;
		unsigned int param2 = AREA->reg_a2;
		unsigned int param3 = AREA->reg_a3;

	#endif

	#ifdef TARGET_UARM
    
    	//Accedo alla Old Area della system call
		cause = CAUSE_EXCCODE_GET(AREA->CP15_Cause);

		unsigned int param0 = AREA->a1;
		unsigned int param1 = AREA->a2;
		unsigned int param2 = AREA->a3;
		unsigned int param3 = AREA->a4;
        
	#endif

    //SYSCALL - EXC_SYSCAL = 8
    if(cause == EXC_SYSCALL){

		//termprint("SYS \n");

		//SYSCALL 1
		if(param0 == GETCPUTIME){
			
			//termprint("SYS 1 \n");
			getCPUTime(&param1, &param2, &param3);

		}

		//SYSCALL 2
		else if(param0 == CREATEPROCESS){
			
			//termprint("SYS 2 \n");
			ritorno =  CreateProcess((state_t*)param1, (int)param2, (void **)param3);			
			

		}

		//SYSCALL 3
		else if(param0 == TERMINATEPROCESS){
			
			
			//termprint("SYS 3 \n");
			ritorno = TerminateProcess((void *)param1);

		}

		//SYSCALL 4
		else if(param0 == VERHOGEN){

			//termprint("SYS 4 \n");
			//insert = TRUE;
			Verhogen((int*)param1);
			
		}

		//SYSCALL 5
		else if(param0 == PASSEREN){
			
			//termprint("SYS 5 \n");
			//insert = TRUE;
			Passeren((int*)param1);

		}

		//SYSCALL 6
		else if(param0 == WAITIO){

			GOODMORNING_PCB=ACTIVE_PCB;	
			//termprint("SYS 6 \n");
			ritorno = DO_IO((unsigned int)param1, (unsigned int*)param2, (int)param3);	
		

		}

		//SYSCALL 7
		else if(param0 == SPECPASSUP){
			
			ritorno = SpecPassup(param1, (state_t *)param2, (state_t *)param3);
			
		}

		//SYSCALL 8
		else if(param0 == GETPID){
			
      		//termprint("SYS 8 \n");
			getPid((void **)param1, (void **)param2);
		
		}
		
		//se la syscall è maggiore di 8
		else{
			
			SavePCBToOldArea((state_t*)SYSBK_OLDAREA, &(ACTIVE_PCB->p_s));
			
			if(ACTIVE_PCB->SysNew != NULL && ACTIVE_PCB->SysOld != NULL){
				//c'è un gestore di livello superiore di tipo TLB, perciò si copia nell'old area lo stato del processo corrente e si carica nel curr_proc il codice della new area.

				SavePCBToOldArea((state_t*)SYSBK_OLDAREA,  (ACTIVE_PCB->SysOld));
				
				//SavePCBToOldArea(ACTIVE_PCB->PTOld, &(ACTIVE_PCB->p_s));
				LDST(ACTIVE_PCB->SysNew);
			
			}
			else{

				//non c'è un puntatore ad un gestore di livello superiore, e quindi il processo va terminato
				TerminateProcess(0);

				ACTIVE_PCB = NULL;

				Scheduling();
				
			}
			
		}

    }

	//BREAKPOINT - 9
	else if(cause == EXC_BP){

		SavePCBToOldArea((state_t*)SYSBK_OLDAREA, &(ACTIVE_PCB->p_s));
		
		if(ACTIVE_PCB->SysNew != NULL && ACTIVE_PCB->SysOld != NULL){
			//c'è un gestore di livello superiore di tipo TLB, perciò si copia nell'old area lo stato del processo corrente e si carica nel curr_proc il codice della new area.*/

			SavePCBToOldArea((state_t*)SYSBK_OLDAREA,  (ACTIVE_PCB->SysOld));
			
			//SavePCBToOldArea(ACTIVE_PCB->PTOld, &(ACTIVE_PCB->p_s));
			LDST(ACTIVE_PCB->SysNew);
		
		}
		else{

			//non c'è un puntatore ad un gestore di livello superiore, e quindi il processo va terminato
			TerminateProcess(0);

			ACTIVE_PCB = NULL;

			Scheduling();
			
		}

	}

	//Ho un processo ancora attivo in cpu
	if(ACTIVE_PCB != NULL){

		//Salvo lo stato
		SavePCBToOldArea(AREA, &(ACTIVE_PCB->p_s));

		#ifdef TARGET_UMPS
		
			ACTIVE_PCB->p_s.reg_v0 = ritorno;

			//Salvo il valore del tempo in kernel mode perchè sto entrando in user mode 
			ACTIVE_PCB->kernel_total += (getTODLO() - ACTIVE_PCB->kernel_start);

			//Setto
			ACTIVE_PCB->kernel_start = 0;

			//Faccio partire l'user mode perchè finisco un interrupt
			ACTIVE_PCB->user_start = getTODLO();

		#endif

		#ifdef TARGET_UARM

			ACTIVE_PCB->p_s.a1 = ritorno;
		
		#endif

		LDST(&ACTIVE_PCB->p_s);

	}

	//Non ho più processi attivi sulla cpu
	else{
		
		//Chiamo lo scheduler
		Scheduling();
	
	}

}

//Gestore delle trap
void trapHandler(){

	unsigned int cause;

	state_t *AREA = (state_t *) PGMTRAP_OLDAREA;

	int flag = FALSE;

	#ifdef TARGET_UMPS
		
		//Aumentiamo di una word
		//AREA->pc_epc = AREA->pc_epc + 4;
		
		//Accedo alla Old Area della system call
		cause = (CAUSE_GET_EXCCODE(AREA->cause));

		//UMPS: 4-5-6-7-10-11-12
		if(cause==EXC_ADDRINVLOAD || cause==EXC_ADDRINVSTORE || cause==EXC_BUSINVFETCH || cause==EXC_BUSINVLDSTORE || cause==EXC_RESERVEDINSTR || cause==EXC_COPROCUNUSABLE || cause==EXC_ARITHOVERFLOW){

			flag = TRUE;
		
		}
	
	#endif

	#ifdef TARGET_UARM
    
    	//Accedo alla Old Area della system call
		cause = CAUSE_EXCCODE_GET(AREA->CP15_Cause);
		
		//UARM: 16-17-2-1
		if(cause == EXC_ADDRINVLOAD || cause == EXC_ADDRINVSTORE || cause == EXC_BUSINVFETCH || cause == EXC_BUSINVLDSTORE || cause == MEMERROR){

			flag = TRUE;
		
		}

	#endif
	
	//umps 6 uarm 2 okk
	//stampaCauseExc(cause);

	if(flag && ACTIVE_PCB->PTNew != NULL && ACTIVE_PCB->PTOld != NULL){

		SavePCBToOldArea((state_t*)PGMTRAP_OLDAREA, &(ACTIVE_PCB->p_s));
		
			
		
		//c'è un gestore di livello superiore di tipo TLB, perciò si copia nell'old area lo stato del processo corrente e si carica nel curr_proc il codice della new area.*/

		SavePCBToOldArea((state_t*)PGMTRAP_OLDAREA,  (ACTIVE_PCB->PTOld));
		
		//SavePCBToOldArea(ACTIVE_PCB->PTOld, &(ACTIVE_PCB->p_s));
		LDST(ACTIVE_PCB->PTNew);
	

	
	}

	else{
		

		//non c'è un puntatore ad un gestore di livello superiore, e quindi il processo va terminato
		TerminateProcess(0);

		ACTIVE_PCB = NULL;

		Scheduling();
		
	}


}



//TLB HADLER
void tlbHandler(){

	unsigned int cause;

	state_t *AREA = (state_t *) TLB_OLDAREA;

		int flag = FALSE;

	#ifdef TARGET_UMPS
		
		//Aumentiamo di una word
		//AREA->pc_epc = AREA->pc_epc + 4;
		
		//Accedo alla Old Area della system call
		cause = (CAUSE_GET_EXCCODE(AREA->cause));

		//UMPS Exc_CODE: 1-2-3-13-14
		if(cause==EXC_TLBMOD || cause==EXC_TLBINVLOAD || cause==EXC_TLBINVSTORE || cause==EXC_BADPTE || cause==EXC_PTEMISS){

			flag = TRUE;
		
		}

	#endif

	#ifdef TARGET_UARM
    
    	//Accedo alla Old Area della system call
		cause = CAUSE_EXCCODE_GET(AREA->CP15_Cause);

		//UARM Exc_CODE: 18-14-15-10-11-9-8-12-13
		if(cause == EXC_TLBMOD || cause ==  EXC_TLBINVLOAD || cause == EXC_TLBINVSTORE || cause == EXC_BADPTE || cause == EXC_PTEMISS || cause == EXC_BADPAGTBL || cause == EXC_BADSEGTBL || cause == UTLBLEXCEPTION || cause == UTLBSEXCEPTION){

			flag = TRUE;

		}

	#endif
	
	//umps 13 uarm 2 okk
	//stampaCauseExc(cause);

	if(flag && ACTIVE_PCB->TLBNew != NULL && ACTIVE_PCB->TLBOld != NULL){

		SavePCBToOldArea((state_t*)TLB_OLDAREA, &(ACTIVE_PCB->p_s));
		


		
		//c'è un gestore di livello superiore di tipo TLB, perciò si copia nell'old area lo stato del processo corrente e si carica nel curr_proc il codice della new area.*/

		SavePCBToOldArea((state_t*)TLB_OLDAREA,  (ACTIVE_PCB->TLBOld));
		
		//SavePCBToOldArea(ACTIVE_PCB->PTOld, &(ACTIVE_PCB->p_s));
		LDST(ACTIVE_PCB->TLBNew);
		

		
	}
	else{


		//non c'è un puntatore ad un gestore di livello superiore, e quindi il processo va terminato
		TerminateProcess(0);

		ACTIVE_PCB = NULL;

		Scheduling();
		
	}

}