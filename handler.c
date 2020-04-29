#include "include/handler.h"

#define INTERRUPT_PENDING_MASK     	0x0000ff00
#define INTERRUPT_PENDING_B      	8
#define INTERRUPT_PENDING_FUNC(x)   (((x) & INTERRUPT_PENDING_MASK) >> INTERRUPT_PENDING_B)

#define INT_IP_GET(cause) ((cause >> 8) & 0xFF)

//PER UARM VEDI FILE UARMcost.h riga 164
//#define CAUSE_IP_GET(cause, int_no) ((cause) & (1 << ((int_no) + 24)))

//Gestore degli interrupt
void interruptHandler(){
	
	// salvo il valore del tempo in kernelmode perchè sto entrando in user mode 
	//ACTIVE_PCB->user_total += getTODLO() - ACTIVE_PCB->user_start;
	//inizio a contare il tempo in user mode
	//ACTIVE_PCB->kernel_start = getTODLO();


	//Salvo i registri dell'old area dell'interrupt al processo
	struct state *AREA=(state_t *) INT_OLDAREA;

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


	if(INT_IP_GET(cause) == INT_T_SLICE){
		
		termprint("Interrupt: 1 \n");
	}else if(INT_IP_GET(cause) == INT_TIMER){
		termprint("Interrupt: 2 \n");

		InterruptIntervalTimer();
	}else if(INT_IP_GET(cause) == INT_DISK){
		termprint("Interrupt: 3 \n");
	}else if(INT_IP_GET(cause) == INT_TAPE){
		termprint("Interrupt: 4 \n");
		InterruptTape();
	}else if(INT_IP_GET(cause) == INT_UNUSED){
		termprint("Interrupt: 5 \n");
	}else if(INT_IP_GET(cause) == INT_PRINTER){
		termprint("Interrupt: 6 \n");
	}else if(INT_IP_GET(cause) == INT_TERMINAL){
		termprint("Interrupt: 7 \n");
	}else{
		termprint("Interrupt: err\n");
	}

    //Linee interrupt da confrontare per trovare l'interrupt giusto fra gli 8 possibili
    
	// //Interrupt 1 - Inter-processor interrupts
	// if(INTERRUPT_PENDING_FUNC(cause) == INT_T_SLICE){

	// 	termprint("Interrupt: 1 \n");
	
	// }
	// //Interrupt 2 Interval Timer
	// else if(INTERRUPT_PENDING_FUNC(cause) == INT_TIMER){
		
	// 	termprint("Interrupt: 2 \n");
	// 	InterruptIntervalTimer();
	// 	Scheduling();
	
	// }
	// //Interrut 3 - Disk
	// else if(INTERRUPT_PENDING_FUNC(cause) == INT_DISK){
	
	// 	termprint("Interrupt: 3 \n");
	
	// }
	// //Interrupt 4 - Tape
	// else if(INTERRUPT_PENDING_FUNC(cause) == INT_TAPE){
	
	// 	termprint("Interrupt: 4 \n");
	
	// }
	// //Interrupt 5 - Network
	// else if(INTERRUPT_PENDING_FUNC(cause) == INT_UNUSED){
	
	// 	termprint("Interrupt: 5 \n");
	
	// }
	// //Interrupt 6 - Printer
	// else if(INTERRUPT_PENDING_FUNC(cause) == INT_PRINTER){
	
	// 	termprint("Interrupt: 6 \n");
	
	// }
	// //Interrupt 7 - Terminal
	// else if(INTERRUPT_PENDING_FUNC(cause) == INT_TERMINAL){
		
	// 	termprint("Interrupt: 7 \n");
	
	// }
	
	// else{
		
	// 	termprint("Interrupt: non so in che interrupt sono\n");
	
	// }

	//Inviamo ACK a CP0
  	//*(unsigned int*)BUS_REG_TIMER = TIME_SLICE;


	//Richiamo lo scheduler
	Scheduling();

}

//====================================================================================================================

//Gestore delle system call
void syscallHandler(){

	//insert = FALSE;

	//Metti in pausa il contatore del tempo del pcb e aggiorni il suo valore 

	// struct state *AREA;
	unsigned int cause;
	int flag = 0; //Setto a true se devo ritornare qualcosa
	unsigned int ritorno; //Assegno il valore di ritorno

	

	struct state *AREA = (state_t *) SYSBK_OLDAREA;
	
  	#ifdef TARGET_UMPS

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
			
			termprint("SYS 1 \n");
			getCPUTime(&AREA->reg_a1, &AREA->reg_a2, &AREA->reg_a3);

		}

		//SYSCALL 2
		else if(AREA->reg_a0 == CREATEPROCESS){
			
			termprint("SYS 2 \n");
			ritorno =  CreateProcess((struct state*)AREA->reg_a1, (int)AREA->reg_a2, (void **)AREA->reg_a3);			
			flag = 1;	

		}

		//SYSCALL 3
		else if(AREA->reg_a0 == TERMINATEPROCESS){
			
			termprint("SYS 3 \n");
			ritorno = TerminateProcess((void **)AREA->reg_a1);
			flag = 1;	

		}

		//SYSCALL 4
		else if(AREA->reg_a0 == VERHOGEN){

			termprint("SYS 4 \n");
			//insert = TRUE;
			Verhogen((int*)AREA->reg_a1);
			
		}

		//SYSCALL 5
		else if(AREA->reg_a0 == PASSEREN){
			
			termprint("SYS 5 \n");
			//insert = TRUE;
			Passeren((int*)AREA->reg_a1);
		
		}

		//SYSCALL 6
		else if(AREA->reg_a0 == WAITIO){
			
			termprint("SYS 6 \n");
			ritorno = DO_IO((unsigned int)AREA->reg_a1, (unsigned int*)AREA->reg_a2, (int)AREA->reg_a3);	
			flag = 1;	

		}

		//SYSCALL 7
		else if(AREA->reg_a0 == SPECPASSUP){
			
			termprint("SYS 7 \n");
			ritorno = SpecPassup(AREA->reg_a1, (struct state*)AREA->reg_a2, (struct state*)AREA->reg_a3);
			flag = 1;
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

		if (flag == 1){

			#ifdef TARGET_UMPS
				AREA->reg_v0 = ritorno;
			#endif

			#ifdef TARGET_UARM
				AREA->reg_a0 = ritorno;
			#endif
		}

    }

	// //Controllo che sia un Breakpoint (EXC_BP 9)
    // else if(CAUSE_GET_EXCCODE(AREA->cause) == EXC_BP){
    
    // 	//termprint("E' partito un Breakpoint \n");
    
    // }

	#ifdef TARGET_UMPS
    	
		state_t* oldarea = ((state_t*)SYSCALL_OLDAREA);

		//dopo aver invocato una system call e’ necessario incrementare il program counter di una word affinché il processo continui
		oldarea->pc_epc = oldarea->pc_epc + 4;

	#endif

	//Ho un processo ancora attivo in cpu
	if(ACTIVE_PCB != NULL){

		termprint("Sys: carico processo in CPU \n");
		
		LDST(&ACTIVE_PCB->p_s);

	}

	//Non ho più processi attivi sulla cpu
	else{

		termprint("Sys: vado nello scheduler \n");

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

