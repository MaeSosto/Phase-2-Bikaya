#include "include/handler.h"
#include "include/utils.h"
#include "include/pcb.h"
#include "include/types_bikaya.h"
#include "include/const_bikaya.h"

#ifdef TARGET_UMPS

  #include <umps/cp0.h>
  #include <umps/types.h>
  #include <umps/libumps.h>
  #include <umps/arch.h>

  //DEFINE
  #define FRAMESIZE 4096
  #define RAMBASE    *((unsigned int *)BUS_REG_RAM_BASE)
  #define RAMSIZE    *((unsigned int *)BUS_REG_RAM_SIZE)
  #define RAMTOP     (RAMBASE + RAMSIZE) //-> presa da test1.5 bikaya

  //NEW-OLD AREA UMPS
  #define SYSCALL_NEWAREA 0x200003D4
  #define SYSCALL_OLDAREA 0x20000348
  #define TRAP_NEWAREA 0x200002BC
  #define TLB_NEWAREA 0x200001A4
  #define INTERRUPT_NEWAREA 0x2000008C
  #define INTERRUPT_OLDAREA 0x20000000

  //Numero massimo di linee/device che posso avere
  #define MAX_DEVICES (DEV_USED_INTS * DEV_PER_INT) + DEV_PER_INT + 1

#endif

#ifdef TARGET_UARM

  #include <uarm/arch.h>
  #include <uarm/uARMtypes.h>
  #include <uarm/uARMconst.h>
  #include <uarm/libuarm.h>

#endif

#define BUS_TODLOW  0x1000001c
#define BUS_TODHIGH 0x10000018
#define getTODLO() (*((unsigned int *)BUS_TODLOW))

extern struct pcb_t *ACTIVE_PCB;
extern struct list_head* ready_queue;
extern struct device_semd Semaforo;
extern int SemMem[MAX_DEVICES];
extern void termprint(char *str);

//Inizializzo le Areas
void setAreas(){

  #ifdef TARGET_UMPS

    state_t *SYSCALL = (state_t*) SYSCALL_NEWAREA;
    SYSCALL->pc_epc = (unsigned int)syscallHandler;
    SYSCALL->reg_sp = RAMTOP; 
    SYSCALL->status = SYSCALL->status | STATUS_IEc;    
    SYSCALL->status = SYSCALL->status & ~STATUS_KUc;
    SYSCALL->status = SYSCALL->status | STATUS_IEp;
    SYSCALL->status = SYSCALL->status & ~STATUS_IM_MASK;
    SYSCALL->status = SYSCALL->status & ~STATUS_VMc;

    state_t *TRAP = (state_t*) TRAP_NEWAREA;
    TRAP->pc_epc = (unsigned int)trapHandler;
    TRAP->reg_sp = RAMTOP; 
    TRAP->status = SYSCALL->status | STATUS_IEc;    
    TRAP->status = TRAP->status & ~STATUS_KUc;
    TRAP->status = TRAP->status | STATUS_IEp;
    TRAP->status = TRAP->status & ~STATUS_IM_MASK;
    TRAP->status = TRAP->status & ~STATUS_VMc;
    
    state_t *TLB = (state_t*) TLB_NEWAREA;
    TLB->pc_epc = (unsigned int)tlbHandler;
    TLB->reg_sp = RAMTOP; 
    TLB->status = TLB->status | STATUS_IEc;    
    TLB->status = TLB->status & ~STATUS_KUc;
    TLB->status = TLB->status | STATUS_IEp;
    TLB->status = TLB->status & ~STATUS_IM_MASK;
    TLB->status = TLB->status & ~STATUS_VMc;
    
    state_t *INTERRUPT = (state_t*) INTERRUPT_NEWAREA;
    INTERRUPT->pc_epc = (unsigned int)interruptHandler;
    INTERRUPT->reg_sp = RAMTOP; 
    INTERRUPT->status = INTERRUPT->status | STATUS_IEc;    
    INTERRUPT->status = INTERRUPT->status & ~STATUS_KUc;
    INTERRUPT->status = INTERRUPT->status | STATUS_IEp;
    INTERRUPT->status = INTERRUPT->status & ~STATUS_IM_MASK;
    INTERRUPT->status = INTERRUPT->status & ~STATUS_VMc;

  #endif
  
  #ifdef TARGET_UARM

    state_t *SYSCALL = (state_t*) SYSBK_NEWAREA;
    SYSCALL->pc = (unsigned int)syscallHandler;  //assegno la funzione che gestisce la syscall
    SYSCALL->sp = RAM_TOP;  //setto ramtop
    SYSCALL->cpsr = STATUS_DISABLE_INT(SYSCALL->cpsr); //Disabilito gli interrupt
    SYSCALL->cpsr = STATUS_DISABLE_TIMER(SYSCALL->cpsr);
    SYSCALL->cpsr = SYSCALL->cpsr | STATUS_SYS_MODE; 
    SYSCALL->CP15_Control = CP15_DISABLE_VM(SYSCALL->CP15_Control); //disabilito virtual memory

    state_t *TRAP = (state_t*) PGMTRAP_NEWAREA;
    TRAP->pc = (unsigned int)trapHandler;
    TRAP->sp = RAM_TOP;
    TRAP->cpsr = STATUS_DISABLE_INT(TRAP->cpsr);
    TRAP->cpsr = STATUS_DISABLE_TIMER(TRAP->cpsr);
    TRAP->cpsr = TRAP->cpsr | STATUS_SYS_MODE; 
    TRAP->CP15_Control = CP15_DISABLE_VM(TRAP->CP15_Control);

    state_t *TLB = (state_t*) TLB_NEWAREA;
    TLB->pc = (unsigned int)tlbHandler;
    TLB->sp = RAM_TOP;
    TLB->cpsr = STATUS_DISABLE_INT(TLB->cpsr);
    TLB->cpsr = STATUS_DISABLE_TIMER(TLB->cpsr);
    TLB->cpsr = TLB->cpsr | STATUS_SYS_MODE; 
    TLB->CP15_Control = CP15_DISABLE_VM(TLB->CP15_Control);

    state_t *INTERRUPT = (state_t*) INT_NEWAREA;
    INTERRUPT->pc = (unsigned int)interruptHandler;
    INTERRUPT->sp = RAM_TOP;
    INTERRUPT->cpsr = STATUS_DISABLE_INT(INTERRUPT->cpsr);
    INTERRUPT->cpsr = STATUS_DISABLE_TIMER(INTERRUPT->cpsr);
    INTERRUPT->cpsr = INTERRUPT->cpsr | STATUS_SYS_MODE; 
    INTERRUPT->CP15_Control = CP15_DISABLE_VM(INTERRUPT->CP15_Control);

  #endif
}

//Inizializzo i Pcb
struct pcb_t *initAllPCB(unsigned int functionAddress, int priority){
	
	int n = priority;
	struct pcb_t *tempPcb = allocPcb();
	
	#ifdef TARGET_UMPS  
		tempPcb->p_s.status = tempPcb->p_s.status | STATUS_IEp;
		tempPcb->p_s.status = tempPcb->p_s.status | STATUS_IEc;
		tempPcb->p_s.status = tempPcb->p_s.status & ~STATUS_KUp;
		tempPcb->p_s.status = tempPcb->p_s.status & ~STATUS_VMp;
		tempPcb->p_s.status = tempPcb->p_s.status | STATUS_IM(2);
		tempPcb->p_s.reg_sp = RAMTOP - FRAMESIZE * n; 
		tempPcb->p_s.pc_epc = functionAddress;   
	#endif

	#ifdef TARGET_UARM
		tempPcb->p_s.cpsr = STATUS_DISABLE_INT(tempPcb->p_s.cpsr); //Enable interrupt (IL PROF DICE DI FAR DISABLE)
		tempPcb->p_s.cpsr = STATUS_ENABLE_TIMER(tempPcb->p_s.cpsr); //Enable timer
		tempPcb->p_s.cpsr = tempPcb->p_s.cpsr | STATUS_SYS_MODE; 
		tempPcb->p_s.CP15_Control = CP15_DISABLE_VM(tempPcb->p_s.CP15_Control); //Disable VM
		tempPcb->p_s.sp = RAM_TOP - FRAMESIZE * n;
		tempPcb->p_s.pc = functionAddress;
    
	#endif

	tempPcb->priority = n;
	tempPcb->original_priority = n;
  tempPcb->wallclock_start=getTODLO();
	return tempPcb;
  
}

//Salvo lo stato della interrupt old area nel processo appena eseguito
void SaveOldState(state_t* oldarea, state_t* processo){
	
	#ifdef TARGET_UMPS
	
		processo->entry_hi = oldarea->entry_hi;
		processo->cause = oldarea->cause;
		processo->status = oldarea->status;
		processo->pc_epc = oldarea->pc_epc;
		processo->hi = oldarea->hi;
		processo->lo = oldarea->lo;
		for(int i=0;i<29;i++){processo->gpr[i]=oldarea->gpr[i];}
		
	#endif
	
	#ifdef TARGET_UARM
	
		processo->a1 = oldarea->a1;
		processo->a2 = oldarea->a2;
		processo->a3 = oldarea->a3;
		processo->a4 = oldarea->a4;
		processo->v1 = oldarea->v1;
		processo->v2 = oldarea->v2;
		processo->v3 = oldarea->v3;
		processo->v4 = oldarea->v4;
		processo->v5 = oldarea->v5;
		processo->v6 = oldarea->v6;
		processo->sl = oldarea->sl;
		processo->fp = oldarea->fp;
		processo->ip = oldarea->ip;
		processo->sp = oldarea->sp;
		processo->lr = oldarea->lr;
		processo->pc = oldarea->pc;
		processo->cpsr = oldarea->cpsr;
		processo->CP15_Control = oldarea->CP15_Control;
		processo->CP15_EntryHi = oldarea->CP15_EntryHi;
		processo->CP15_Cause = oldarea->CP15_Cause;
		processo->TOD_Hi = oldarea->TOD_Hi;
		processo->TOD_Low = oldarea->TOD_Low;

	#endif
	
}


//Alloca spazio in memoria per i semafori dei device
void InitSemd(){

	//inizializzo a 0 le celle della matrice che gestisce i semd
	for(int i=0; i<MAX_DEVICES; i++){

		SemMem[i] = 0;

	}

  //Assegno gli indirizzi di memoria ai registri dei device
	for(int i=0; i<DEV_PER_INT; i++){
		
		Semaforo.disk[i].s_key = &SemMem[i]; //Da 0 a 7
		Semaforo.tape[i].s_key = &SemMem[i+DEV_PER_INT]; //Da 8 a 15
		Semaforo.network[i].s_key = &SemMem[i+2*DEV_PER_INT]; //Da 16 a 23
		Semaforo.printer[i].s_key = &SemMem[i+3*DEV_PER_INT]; //Da 24 a 31
		Semaforo.terminal[i].s_key = &SemMem[i+4*DEV_PER_INT]; //Da 32 a 39
		
	}

}

//Qualsiasi sia l'indirizzo passato (di una qualsiasi linea), restituisce un numero da 0 a 7 (quale device)
int numDev(unsigned int *registro){

	for(int i = 0; i < DEV_PER_INT; i++){

		for(int j = 0; j < DEV_PER_INT; j++){

			if((unsigned int*)DEV_REG_ADDR(i, j) == registro){
				
				return j;

			}

		}
	
	}

	return -1;

}

//Qualsiasi sia l'indirizzo passato, restituisce un numero da 3 a 7 (quale linea)
int numLine(unsigned int *registro){
	
	for(int i = 0; i < DEV_PER_INT; i++){

		for(int j = 0; j < DEV_PER_INT; j++){

			if((unsigned int*)DEV_REG_ADDR(i, j) == registro){
				
				return i;

			}

		}
	
	}

	return -1;

}

void stampaInt(int n){
	
	if(n < 0) termprint("ERRORE \n");
	if(n == 0) termprint("ZERO \n");
	if(n == 1) termprint("UNO \n");
	if(n == 2) termprint("DUE \n");
	if(n == 3) termprint("TRE \n");
	if(n == 4) termprint("QUATTRO \n");
	if(n == 5) termprint("CINQUE \n");
	if(n == 6) termprint("SEI \n");
	if(n == 7) termprint("SETTE \n");
	if(n == 8) termprint("OTTO \n");
	if(n > 8) termprint("ALTRO \n");

}