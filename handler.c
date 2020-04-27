#include "include/handler.h"
#include "include/scheduler.h"
#include "include/pcb.h"
#include "include/utils.h"
#include "include/syscall.h"
#include "include/const_bikaya.h"

#ifdef TARGET_UMPS

	#include "umps/arch.h"
	#include "umps/cp0.h"
	#include <umps/libumps.h>
	#define INT_OLDAREA 0x20000000
	#define SYSCALL_OLDAREA   0x20000348
	#define SYS3              3
	extern void termprint(char* str);

#endif

#ifdef TARGET_UARM

	#include <uarm/uARMconst.h>
	#include <uarm/uARMtypes.h>
	#include <uarm/arch.h>
	#include <uarm/libuarm.h>
	extern void tprint(char* str);

#endif

#define TIME_SLICE 3000

#define BUS_TODLOW  0x1000001c
#define BUS_TODHIGH 0x10000018
#define getTODLO() (*((unsigned int *)BUS_TODLOW))

extern struct pcb_t *ACTIVE_PCB;
extern struct list_head* ready_queue;
int insert = FALSE;

//Gestore degli interrupt
void interruptHandler(){
	termprint("sono dentro un interrupttttt");
	// salvo il valore del tempo in kernelmode perchè sto entrando in user mode 
	ACTIVE_PCB->user_total += getTODLO() - ACTIVE_PCB->user_start;
	//inizio a contare il tempo in user mode
	ACTIVE_PCB->kernel_start = getTODLO();

	//Salvo i registri dell'old area dell'interrupt al processo 
	state_t* oldarea = ((state_t*)INT_OLDAREA);

	#ifdef TARGET_UARM

		//Decremento il program counter dell'interrupt old area di una word
		oldarea->pc = oldarea->pc - 4;

	#endif

	//Inviamo ACK a CP0
	*(unsigned int*)BUS_REG_TIMER = TIME_SLICE;

	//Aging
	Aging();

	//Ripristiniamo l'original_priority del processo appena concluso
	ACTIVE_PCB->priority = ACTIVE_PCB->original_priority;

	//Copio lo stato della old area dell'intertupt nel processo che lo ha sollevato 
	SaveOldState(oldarea, &(ACTIVE_PCB->p_s));

	//Rimetto il processo in attesa nella Ready Queue
	insertProcQ(ready_queue, ACTIVE_PCB);

	//Rientro nello scheduler
	Scheduling();

}


//Gestore delle system call
void syscallHandler(){

	insert = FALSE;

	//Metti in pausa il contatore del tempo del pcb e aggiorni il suo valore 

	struct state *AREA;
	unsigned int cause;
	int flag = 0; //Setto a true se devo ritornare qualcosa
	unsigned int ritorno; //Assegno il valore di ritorno

  	#ifdef TARGET_UMPS

		//Accedo alla Old Area della system call
		AREA=(struct state *) SYSCALL_OLDAREA;
		cause = (CAUSE_GET_EXCCODE(AREA->cause));
	
	#endif

	#ifdef TARGET_UARM
    
    	//Accedo alla Old Area della system call
    	AREA = (state_t *) SYSBK_OLDAREA;
		cause = CAUSE_EXCCODE_GET(AREA->CP15_Cause);
        
	#endif

    //SYSCALL
    if(cause == EXC_SYSCALL){

		termprint("SYS \n");

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
			insert = TRUE;
			Verhogen((int*)AREA->reg_a1);
			
		}

		//SYSCALL 5
		else if(AREA->reg_a0 == PASSEREN){
			
			termprint("SYS 5 \n");
			insert = TRUE;
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

	//Controllo che sia un Breakpoint (EXC_BP 9)
    else if(CAUSE_GET_EXCCODE(AREA->cause) == EXC_BP){
    
      //termprint("E' partito un Breakpoint \n");
    
    }

	//Chiamo lo scheduler
	Scheduling();

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