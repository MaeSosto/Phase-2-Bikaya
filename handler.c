#include "include/handler.h"
#include "include/scheduler.h"
#include "include/pcb.h"
#include "include/utils.h"

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

extern struct pcb_t *ACTIVE_PCB;
extern struct list_head* ready_queue;

//Gestore degli interrupt
void interruptHandler(){

  //Salvo i registri dell'old area dell'interrupt al processo 
	state_t* oldarea = ((state_t*)INT_OLDAREA);

  #ifdef TARGET_UARM

		//Decremento il program counter dell'interrupt old area di una word
		oldarea->pc = oldarea->pc - 4;

	#endif
	
	/* Copio lo stato della old area dell'intertupt nel processo che lo ha sollevato */
	SaveOldState(oldarea, &(ACTIVE_PCB->p_s));

  //Inviamo ACK a CP0
  *(unsigned int*)BUS_REG_TIMER = TIME_SLICE;

  Scheduling();

  
}

//Gestore delle system call
void syscallHandler(){
  
  #ifdef TARGET_UMPS

    //Accedo alla Old Area della system call
    struct state *AREA=(struct state *) SYSCALL_OLDAREA;

    //Controllo che sia una Systemcall (EXC_SYS 8)
    if((CAUSE_GET_EXCCODE(AREA->cause)) == EXC_SYS){

      if(AREA->reg_a0 == SYS3){ //Se a0 mi dice che la system call lanciata è la numero 3 allora: Quando invocata, la SYS3 termina il processo corrente e tutta la sua progenie, rimuovendoli dalla Ready Queue.
        
        freePcb(ACTIVE_PCB); //Termino il processo corrente e lo libero e lo metto nella free pcb
        
        ACTIVE_PCB = NULL;

        Scheduling();
      
      }
    
    }
    
    //Controllo che sia un Breakpoint (EXC_BP 9)
    else if(CAUSE_GET_EXCCODE(AREA->cause) == EXC_BP){
    
      tprint("E' partito un Breakpoint \n");
    
    }

  #endif

  #ifdef TARGET_UARM
    
    //Accedo alla Old Area della system call
    state_t *AREA = (state_t *) SYSBK_OLDAREA;

    //Controllo che sia una Systemcall (EXC_SYS 8)
    if((CAUSE_EXCCODE_GET(AREA->CP15_Cause)) == EXC_SYSCALL){
        
      freePcb(ACTIVE_PCB); //Termino il processo corrente e lo libero e lo metto nella free pcb

      ACTIVE_PCB = NULL;

      Scheduling();
      
    }

    //Controllo che sia un Breakpoint (EXC_BP 9)
    else if(CAUSE_EXCCODE_GET(AREA->CP15_Cause) == EXC_BREAKPOINT){
    
      tprint("E' partito un Breakpoint \n");
    
    }
  
  #endif  

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