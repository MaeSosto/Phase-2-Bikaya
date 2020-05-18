#include "include/utils.h"


//Inizializzo le new area
void setAreas(){

	#ifdef TARGET_UMPS

		state_t *SYSCALL = (state_t*) SYSBK_NEWAREA;
		SYSCALL->pc_epc = (unsigned int)syscallHandler;
		SYSCALL->reg_sp = RAMTOP; 
		SYSCALL->status = SYSCALL->status & ~STATUS_IEc;    	//Tutti gli interrupt disabilitati (settati a 0)
		SYSCALL->status = SYSCALL->status & ~STATUS_IEp;		//Disabilito IEp
		SYSCALL->status = SYSCALL->status & ~STATUS_IM_MASK; 	//Disabilito maschera interrupt
		SYSCALL->status = SYSCALL->status & ~STATUS_KUc;		//Abilito kernel mode KUc=0 OK
		SYSCALL->status = SYSCALL->status & ~STATUS_VMc;		//Disabilito virtual memory OK
		SYSCALL->status = SYSCALL->status | STATUS_CU0;			//Abilito il 28-esimo bit CU[0]

		state_t *TRAP = (state_t*) PGMTRAP_NEWAREA;
		TRAP->pc_epc = (unsigned int)trapHandler;
		TRAP->reg_sp = RAMTOP; 
		TRAP->status = TRAP->status & ~STATUS_IEc;    	//Tutti gli interrupt disabilitati   
		TRAP->status = TRAP->status & ~STATUS_IEp;		//Disabilito IEp
		TRAP->status = TRAP->status & ~STATUS_IM_MASK; 	//Disabilito maschera interrupt
		TRAP->status = TRAP->status & ~STATUS_KUc;		//Abilito kernel mode KUc=0 OK
		TRAP->status = TRAP->status & ~STATUS_VMc;		//Disabilito virtual memory OK
		TRAP->status = TRAP->status | STATUS_CU0;

		state_t *TLB = (state_t*) TLB_NEWAREA;
		TLB->pc_epc = (unsigned int)tlbHandler;
		TLB->reg_sp = RAMTOP; 
		TLB->status = TLB->status & ~STATUS_IEc;    	//Tutti gli interrupt disabilitati   
		TLB->status = TLB->status & ~STATUS_IEp;		//Disabilito IEp
		TLB->status = TLB->status & ~STATUS_IM_MASK; 	//Disabilito maschera interrupt
		TLB->status = TLB->status & ~STATUS_KUc;		//Abilito kernel mode KUc=0 OK
		TLB->status = TLB->status & ~STATUS_VMc;		//Disabilito virtual memory OK
		TLB->status = TLB->status | STATUS_CU0;

		state_t *INTERRUPT = (state_t*) INTERRUPT_NEWAREA;
		INTERRUPT->pc_epc = (unsigned int)interruptHandler;
		INTERRUPT->reg_sp = RAMTOP; 
		INTERRUPT->status = INTERRUPT->status & ~STATUS_IEc;    	//Tutti gli interrupt disabilitati   
		INTERRUPT->status = INTERRUPT->status & ~STATUS_IEp;		//Disabilito IEp
		INTERRUPT->status = INTERRUPT->status & ~STATUS_IM_MASK; 	//Disabilito maschera interrupt
		INTERRUPT->status = INTERRUPT->status & ~STATUS_KUc;		//Abilito kernel mode KUc=0 OK
		INTERRUPT->status = INTERRUPT->status & ~STATUS_VMc;		//Disabilito virtual memory OK
		INTERRUPT->status = INTERRUPT->status | STATUS_CU0;

	#endif
  
	#ifdef TARGET_UARM
	

	state_t *SYSCALL = (state_t*) SYSBK_NEWAREA;
    SYSCALL->pc = (unsigned int)syscallHandler;							//Assegno l'indirizzo all'handler che gestirà l'eccezione quando verrà sollevata
    SYSCALL->sp = RAM_TOP;  											//setto ramtop
    SYSCALL->cpsr = STATUS_DISABLE_INT(SYSCALL->cpsr); 					//Disabilito gli interrupt
    SYSCALL->cpsr = STATUS_DISABLE_TIMER(SYSCALL->cpsr);
    SYSCALL->cpsr = SYSCALL->cpsr | STATUS_SYS_MODE; 
    SYSCALL->CP15_Control = CP15_DISABLE_VM(SYSCALL->CP15_Control);		//Disabilito virtual memory

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

//Inizializzo i pcb
struct pcb_t *initAllPCB(unsigned int functionAddress, int priority){
	
	int n = priority;
	struct pcb_t *tempPcb = allocPcb();

	#ifdef TARGET_UMPS  
		//Bit settati a 1
		tempPcb->p_s.status = tempPcb->p_s.status | STATUS_IEp;		//Abilito IEp
		tempPcb->p_s.status = tempPcb->p_s.status | STATUS_IEc;
		tempPcb->p_s.status = tempPcb->p_s.status | STATUS_IM_MASK;	//Abilito la maschera
		tempPcb->p_s.status = tempPcb->p_s.status | STATUS_CU0;		//Abilito il 28-esimo bit CU[0]
		tempPcb->p_s.status = tempPcb->p_s.status | STATUS_IM(2);	//Abilito IT

		//Bit settati a 0
		tempPcb->p_s.status = tempPcb->p_s.status & ~STATUS_KUc;	//Abilito kernel mode KUc = 0
		tempPcb->p_s.status = tempPcb->p_s.status & ~STATUS_VMc;	//Disabilito virtual memory (24-esimo bit)		
		tempPcb->p_s.status = tempPcb->p_s.status & ~STATUS_IM(7);	//Disabilito interrupt del terminale (15-esimo bit
		tempPcb->p_s.status = tempPcb->p_s.status & ~STATUS_KUp;	//Abilito kernel mode precedente KUp = 0
		tempPcb->p_s.status = tempPcb->p_s.status & ~STATUS_VMp;	//Disabilito virtual memory precedente
		tempPcb->p_s.reg_sp = RAMTOP - FRAMESIZE * n; 
		tempPcb->p_s.pc_epc = functionAddress;   
	#endif

	#ifdef TARGET_UARM

		tempPcb->p_s.cpsr = STATUS_ENABLE_TIMER(tempPcb->p_s.cpsr); 			//Enable timer
		tempPcb->p_s.cpsr = STATUS_ENABLE_INT(tempPcb->p_s.cpsr); 				//Enable interrupt
		tempPcb->p_s.cpsr = tempPcb->p_s.cpsr | STATUS_SYS_MODE; 
		tempPcb->p_s.CP15_Control = CP15_DISABLE_VM(tempPcb->p_s.CP15_Control); //Disable VM
		tempPcb->p_s.sp = RAM_TOP - FRAMESIZE * n;
		tempPcb->p_s.pc = functionAddress;
		
	#endif

	tempPcb->priority = n;
	tempPcb->original_priority = n;

	return tempPcb;
  
}


//Salvo lo stato del PCB nella old area
void SaveState(state_t* processo, state_t* oldarea){

	#ifdef TARGET_UMPS
	
		oldarea->entry_hi = processo->entry_hi;
		oldarea->cause = 	processo->cause;
		oldarea->status = 	processo->status;
		oldarea->pc_epc = 	processo->pc_epc;
		oldarea->hi = 		processo->hi;
		oldarea->lo = 		processo->lo;
		for(int i=0;i<29;i++){
			
			oldarea->gpr[i]=processo->gpr[i];
		                                                                                                                                                                                                   
		}
		
	#endif
	
	#ifdef TARGET_UARM
	
		oldarea->a1 = 			processo->a1; 
		oldarea->a2 = 			processo->a2;
		oldarea->a3 = 			processo->a3;
		oldarea->a4 = 			processo->a4;
		oldarea->v1 = 			processo->v1;
		oldarea->v2 = 			processo->v2;
		oldarea->v3 = 			processo->v3;
		oldarea->v4 = 			processo->v4;
		oldarea->v5 = 			processo->v5;
		oldarea->v6 = 			processo->v6;
		oldarea->sl = 			processo->sl;
		oldarea->fp = 			processo->fp;
		oldarea->ip = 			processo->ip;
		oldarea->sp = 			processo->sp;
		oldarea->lr = 			processo->lr;
		oldarea->pc = 			processo->pc;
		oldarea->cpsr = 		processo->cpsr;
		oldarea->CP15_Control = processo->CP15_Control;
		oldarea->CP15_EntryHi = processo->CP15_EntryHi;
		oldarea->CP15_Cause = 	processo->CP15_Cause;
		oldarea->TOD_Hi = 		processo->TOD_Hi;
		oldarea->TOD_Low = 		processo->TOD_Low;

	#endif

}


//Alloca dello spazio in memoria per i semafori dei device
void InitSemd(){

	//Inizializzo a 0 le celle della matrice che gestisce i semd
	for(int i=0; i<MAX_SEM; i++){

		SemMem[i] = 0;

	}

  	//Assegno gli indirizzi di memoria ai registri dei device
	for(int i=0; i<9; i++){
		
		Semaforo.disk[i].s_key = &SemMem[i]; 						//Da 0 a 7
		Semaforo.tape[i].s_key = &SemMem[i+DEV_PER_INT]; 			//Da 8 a 15
		Semaforo.network[i].s_key = &SemMem[i+2*DEV_PER_INT]; 		//Da 16 a 23
		Semaforo.printer[i].s_key = &SemMem[i+3*DEV_PER_INT]; 		//Da 24 a 31
		Semaforo.terminalR[i].s_key = &SemMem[i+4*DEV_PER_INT]; 	//Da 32 a 39
		Semaforo.terminalT[i].s_key = &SemMem[i+5*DEV_PER_INT]; 	//Da 40 a 47

	}

}

//Funzione che dato in input il registro del device, restituisce il numero del device (da 0 a 7)
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

//Funzione che dato in input il registro del device, restituisce il numero della linea (da 3 a 7)
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

//Restituisce 1 se l'eccezione è stata lanciata dalla linea e dal device dati in input
int Eccezione(int linea, int device){

	if(*INTR_CURRENT_BITMAP(linea) & (1 << device)){

		return 1;

	}

	else{

		return 0;

	}

}
					
//Funzione che controlla se pcbProgenie sta nella progenie del padre
int isChild(pcb_t *padre, pcb_t *pcbProgenie){

	int appoggio = 0;

	//Caso base: in cui il pcb ha padre null 
	if(pcbProgenie->p_parent == NULL){
		
		return 0;
	
	}

	//Caso base: ho trovato il padre
	if(padre == pcbProgenie->p_parent){

		//Ho trovato 
		return 1;

	}
	
	//Casoricorsivo: il pcbProgenie potrebbe essere un nipote
	else{

		return appoggio || isChild(padre, pcbProgenie->p_parent);

	}	

}

//Ritorna il puntatore del primo pcb nella lista dei figli di padre, senza rimuoverlo
struct pcb_t *returnFirstChild(struct pcb_t *padre){
	
	//Caso in cui padre non ha figli, ritorno NULL
	if(emptyChild(padre)) return NULL;

	//Caso in cui padre ha figli, restituisco il puntatore del primo
	struct pcb_t *primofiglio = container_of(list_next(&padre->p_child),struct pcb_t, p_sib);
	
	//Restituisce il puntatore
	return primofiglio;

}

//Restituisce il tempo parziale passato da quado ho settato il kernel time
void stopKernelTime(pcb_t * p){

	if(p->kernel_start > 0){

		p->kernel_total = p->kernel_total + (getTODLO() - p->kernel_start) ;

		p->kernel_start = 0;
		
	}

}

//Il tempo passato in user mode viene archiviato in user total e wallclock e e viene azzerato user start
void stopUserTime(pcb_t *p){

	if(p->user_start > 0){

		p->user_total = p->user_total + (getTODLO() - p->user_start);

		p->user_start = 0;

	}	

}

//Assegno il tempo iniziale
void startWallclockTime(pcb_t *p){

	p->wallclock_start = getTODLO();

}

//Inizia a contare il tempo in kernel mode
void startKernelTime(pcb_t *p){

	p->kernel_start = getTODLO();

}

//Inizia a contare il tempo in user mode
void startUserTime(pcb_t *p){
	
	p->user_start = getTODLO();

}
