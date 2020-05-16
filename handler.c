#include "include/handler.h"

int tempo = FALSE;

//Gestore degli interrupt
void interruptHandler(){
	
	//Gestione del tempo
	if(ACTIVE_PCB != NULL){

		stopUserTime(ACTIVE_PCB);
		stopKernelTime(ACTIVE_PCB);
		startKernelTime(ACTIVE_PCB);

	}

	tempo = FALSE;

	//Prendo l'old area dell'interrupt
	state_t *AREA = (state_t *) INT_OLDAREA;

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

	//Interrupt 1 - Inter-processor interrupts
	#ifdef TARGET_UMPS
	
		if(CAUSE_IP_GET(cause, INT_T_SLICE)){
			
			InterruptPLC();
		
		}

	#endif

	#ifdef TARGET_UARM

		//Interrupt 1 - Inter-processor interrupts 
		//Non presente in un uARM
		if(0){}

	#endif
	
	//Interrupt 2 - Interval Timer
	else if(CAUSE_IP_GET(cause, INT_TIMER)){

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

		InterruptTerminal();

	}


	if(ACTIVE_PCB != NULL){
		
		//gestione tempo
		stopKernelTime(ACTIVE_PCB);

		//Salvo i registri dell'old area dell'interrupt.
		state_t *AREA=(state_t *) INT_OLDAREA;

		//Copio lo stato dell'old area dell'intertupt nel processo che lo ha sollevato
		SaveState(AREA, &(ACTIVE_PCB->p_s));

	}
	
	//Richiamo lo scheduler
	Scheduling();

}


//Gestore delle system call
void syscallHandler(){
	
	unsigned int cause;
	
	//Variabile usata nel caso la system call abbia un valore di ritorno
	unsigned int ritorno; 

	//Gestiso il tempo
	if(ACTIVE_PCB != NULL){

		stopUserTime(ACTIVE_PCB);
		stopKernelTime(ACTIVE_PCB);
		startKernelTime(ACTIVE_PCB);

	}

	state_t *AREA = (state_t *) SYSBK_OLDAREA;
	
	SaveState((state_t *) SYSBK_OLDAREA, &(ACTIVE_PCB->p_s));
	
  	#ifdef TARGET_UMPS
		
		//Aumentiamo il program counter di una word
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
		
		//SYSCALL 1
		if(param0 == GETCPUTIME){
			
			getCPUTime((unsigned int*)param1, (unsigned int*)param2, (unsigned int*)param3);

		}

		//SYSCALL 2
		else if(param0 == CREATEPROCESS){
			
			ritorno =  CreateProcess((state_t*)param1, (int)param2, (void **)param3);		

		}

		//SYSCALL 3
		else if(param0 == TERMINATEPROCESS){
			
			ritorno = TerminateProcess((void *)param1);

		}

		//SYSCALL 4
		else if(param0 == VERHOGEN){

			Verhogen((int*)param1);
			
		}

		//SYSCALL 5
		else if(param0 == PASSEREN){
			
			Passeren((int*)param1);

		}

		//SYSCALL 6
		else if(param0 == WAITIO){

			GOODMORNING_PCB = ACTIVE_PCB;	
			ritorno = DO_IO((unsigned int)param1, (unsigned int*)param2, (int)param3);	
		
		}

		//SYSCALL 7
		else if(param0 == SPECPASSUP){
			
			ritorno = SpecPassup(param1, (state_t *)param2, (state_t *)param3);
			
		}

		//SYSCALL 8
		else if(param0 == GETPID){
			
			getPid((void **)param1, (void **)param2);
		
		}
		
		//Se la syscall è maggiore di 8 passo la gestione della sys al gestore di livello superiore
		else{
			
			//Salvo lo stato
			SaveState((state_t*)SYSBK_OLDAREA, &(ACTIVE_PCB->p_s));
			
			//Controllo se ho un gestore di livello superiore
			if(ACTIVE_PCB->SysNew != NULL && ACTIVE_PCB->SysOld != NULL){

				//Copio nell'old area del processo l'old area della sys
				SaveState((state_t*)SYSBK_OLDAREA,  (ACTIVE_PCB->SysOld));
				
				//Gestione del tempo
				stopKernelTime(ACTIVE_PCB);
				startUserTime(ACTIVE_PCB);

				//Carico la new area del gestore nel processore
				LDST(ACTIVE_PCB->SysNew);
			
			}

			//Non ho un gestore di livello superiore
			else{
				
				//Gestione del tempo
				stopKernelTime(ACTIVE_PCB);

				//Il processo va terminato
				TerminateProcess(0);

				//Non ho più un processo attivo
				ACTIVE_PCB = NULL;

				//Vado allo scheduler
				Scheduling();
				
			}
			
		}

    }

	//BREAKPOINT - 9
	else if(cause == EXC_BP){

		//Salvo lo stato della old area della sys nel processo
		SaveState((state_t*)SYSBK_OLDAREA, &(ACTIVE_PCB->p_s));
		
		//Controllo se ho un gestore di livello superiore
		if(ACTIVE_PCB->SysNew != NULL && ACTIVE_PCB->SysOld != NULL){

			//Copio nell'old area del processo l'old area della sys
			SaveState((state_t*)SYSBK_OLDAREA, (ACTIVE_PCB->SysOld));
			
			//Gestione del tempo
			stopKernelTime(ACTIVE_PCB);
			startUserTime(ACTIVE_PCB);
			
			//Carico la new area del gestore nel processore
			LDST(ACTIVE_PCB->SysNew);
		
		}

		//Non ho un gestore di livello superiore
		else{

			//Gestione del tempo
			stopKernelTime(ACTIVE_PCB);

			//Non c'è un puntatore ad un gestore di livello superiore, e quindi il processo va terminato
			TerminateProcess(0);

			//Non ho più un processo attivo
			ACTIVE_PCB = NULL;

			//Vado allo scheduler
			Scheduling();
			
		}

	}


	//Ho ancora un processo attivo
	if(ACTIVE_PCB != NULL){

		//Salvo lo stato
		SaveState(AREA, &(ACTIVE_PCB->p_s));

		//Setto il ritorno nel registro adeguato
		#ifdef TARGET_UMPS
		
			ACTIVE_PCB->p_s.reg_v0 = ritorno;

		#endif

		#ifdef TARGET_UARM

			ACTIVE_PCB->p_s.a1 = ritorno;
		
		#endif

		//Gestione del tempo
		stopKernelTime(ACTIVE_PCB);
		startUserTime(ACTIVE_PCB);

		//Carico il processo attivo e lo eseguo
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

	//Gestione del tempo
	if(ACTIVE_PCB != NULL){

		stopUserTime(ACTIVE_PCB);
		stopKernelTime(ACTIVE_PCB);
		startKernelTime(ACTIVE_PCB);

	}

	unsigned int cause;

	state_t *AREA = (state_t *) PGMTRAP_OLDAREA;

	int flag = FALSE;

	#ifdef TARGET_UMPS
		
		//Accedo alla old area della trap
		cause = (CAUSE_GET_EXCCODE(AREA->cause));

		//Controllo se viene alzata un exception di questo tipo
		if(cause==EXC_ADDRINVLOAD || cause==EXC_ADDRINVSTORE || cause==EXC_BUSINVFETCH || cause==EXC_BUSINVLDSTORE || cause==EXC_RESERVEDINSTR || cause==EXC_COPROCUNUSABLE || cause==EXC_ARITHOVERFLOW){

			flag = TRUE;
		
		}
	
	#endif

	#ifdef TARGET_UARM
    
    	//Accedo alla Old Area della trap
		cause = CAUSE_EXCCODE_GET(AREA->CP15_Cause);
		
		//Controllo se viene alzata un exception di questo tipo
		if(cause == EXC_ADDRINVLOAD || cause == EXC_ADDRINVSTORE || cause == EXC_BUSINVFETCH || cause == EXC_BUSINVLDSTORE || cause == MEMERROR){

			flag = TRUE;
		
		}

	#endif

	//Controllo se ho un gestore di livello superiore
	if(flag && ACTIVE_PCB->PTNew != NULL && ACTIVE_PCB->PTOld != NULL){
		
		//Salvo lo stato della old area della trap nel processo
		SaveState((state_t*)PGMTRAP_OLDAREA, &(ACTIVE_PCB->p_s));
		
		//Copio nell'old area del processo l'old area della trap
		SaveState((state_t*)PGMTRAP_OLDAREA,  (ACTIVE_PCB->PTOld));

		//Gestione del tempo
		stopKernelTime(ACTIVE_PCB);
		startUserTime(ACTIVE_PCB);
		
		LDST(ACTIVE_PCB->PTNew);
			
	}

	//Non ho un gestore al livello superiore
	else{
		
		//Gestione del tempo
		stopKernelTime(ACTIVE_PCB);

		//Non c'è un puntatore ad un gestore di livello superiore, e quindi il processo va terminato
		TerminateProcess(0);

		//Non ho più un processo attivo
		ACTIVE_PCB = NULL;

		//Vado allo scheduler
		Scheduling();
		
	}


}



//TLB HADLER
void tlbHandler(){

	//Gestione del tempo
	if(ACTIVE_PCB != NULL){

		stopUserTime(ACTIVE_PCB);
		stopKernelTime(ACTIVE_PCB);
		startKernelTime(ACTIVE_PCB);

	}

	unsigned int cause;

	state_t *AREA = (state_t *) TLB_OLDAREA;

		int flag = FALSE;

	#ifdef TARGET_UMPS
		
		//Accedo alla old area della tlb
		cause = (CAUSE_GET_EXCCODE(AREA->cause));

		//Controllo se viene alzata un exception di questo tipo
		if(cause==EXC_TLBMOD || cause==EXC_TLBINVLOAD || cause==EXC_TLBINVSTORE || cause==EXC_BADPTE || cause==EXC_PTEMISS){

			flag = TRUE;
		
		}

	#endif

	#ifdef TARGET_UARM
    
    	//Accedo alla old area della tlb
		cause = CAUSE_EXCCODE_GET(AREA->CP15_Cause);

		//Controllo se viene alzata un exception di questo tipo 
		if(cause == EXC_TLBMOD || cause ==  EXC_TLBINVLOAD || cause == EXC_TLBINVSTORE || cause == EXC_BADPTE || cause == EXC_PTEMISS || cause == EXC_BADPAGTBL || cause == EXC_BADSEGTBL || cause == UTLBLEXCEPTION || cause == UTLBSEXCEPTION){

			flag = TRUE;

		}

	#endif
	
	//Controllo se ho un gestore di livello superiore
	if(flag && ACTIVE_PCB->TLBNew != NULL && ACTIVE_PCB->TLBOld != NULL){

		//Salvo lo stato della old area della tlb nel processo
		SaveState((state_t*)TLB_OLDAREA, &(ACTIVE_PCB->p_s));
		
		//Copio nell'old area del processo l'old area della tlb
		SaveState((state_t*)TLB_OLDAREA,  (ACTIVE_PCB->TLBOld));
		
		//Smetto di essere in kernel mode e passo alla user mode
		stopKernelTime(ACTIVE_PCB);
		startUserTime(ACTIVE_PCB);

		//Cario la new area nel processore 
		LDST(ACTIVE_PCB->TLBNew);
			
	}

	//Non ho un gestore al livello superiore
	else{

		//Gestione del tempo
		stopKernelTime(ACTIVE_PCB);

		//Il processo va terminato
		TerminateProcess(0);

		//Non ho più un processo attivo
		ACTIVE_PCB = NULL;

		//Vado allo scheduler
		Scheduling();
		
	}

}