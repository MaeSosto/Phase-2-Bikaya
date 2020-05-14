#ifndef _UTILS_H_
#define _UTILS_H_

	#include "handler.h"
	#include "pcb.h"
	#include "types_bikaya.h"
	#include "const_bikaya.h"
	#include "scheduler.h"
	
	
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
		//NEW-OLD AREA UMPS
		#define SYSBK_NEWAREA 0x200003D4
		#define SYSBK_OLDAREA 0x20000348
		#define PGMTRAP_NEWAREA 0x200002BC
		#define PGMTRAP_OLDAREA 0x20000230
		#define TLB_OLDAREA 0x20000118
		#define TLB_NEWAREA 0x200001A4
		#define INTERRUPT_NEWAREA 0x2000008C
		#define INTERRUPT_OLDAREA 0x20000
	
	#endif

	#ifdef TARGET_UARM

		#include <uarm/arch.h>
		#include <uarm/uARMtypes.h>
		#include <uarm/uARMconst.h>
		#include <uarm/libuarm.h>

	#endif

	
	#define MAX_SEM 48
	
	extern struct pcb_t *ACTIVE_PCB;
	extern struct list_head* ready_queue;
	extern struct device_semd Semaforo;
	extern int SemMem[MAX_SEM];
	extern void termprint(char *str);

	//Inizializzo le Areas
	void setAreas();

	//Inizializzo i Pcb
	struct pcb_t *initAllPCB(unsigned int functionAddress, int priority);

	//Salvo lo stato del PCB nella old area
	void SavePCBToOldArea(state_t* processo, state_t* oldarea);

	//Salva il processo prima di poterlo mettere di nuovo nella ready queue
	void SaveProc();

	//Alloca spazio in memoria per i semafori dei device
	void InitSemd();

	//Qualsisi sia l'indirizzo passato (di una qualsiasi linea), restituisce un numero da 0 a 7 (quale device)
	int numDev(unsigned int *registro);

	//Qualsiasi sia l'indirizzo passato, restituisce un numero da 3 a 7 (quale linea)
	int numLine(unsigned int *registro);

	//Restituisce 1 se l'eccezione è stata lanciata dal numero della linea e dal device in input
	int Eccezione(int linea, int device);

	//Funzione che controlla se cercare è figlio di padre (o se sta nella sua progenie)
	int isChild(pcb_t *padre, pcb_t *cercare);

	
	/* Funzioni per la gestione del tempo */

	//Restituisce il tempo parziale passato da quado ho settato il kernel time
	void stopKernelTime(pcb_t * p);

	//Il tempo passato in user mode viene archiviato in user total e viene azzerato user start
	void stopUserTime(pcb_t *p);

	//Restituisco il wallclock start
	int getWallclockTime(pcb_t * p);

	//Assegno il tempo iniziale
	void setWallclockTime(pcb_t *p);

	//Inizia a contare il tempo in kernel mode
	void startKernelTime();

	//Inizia a contare il tempo in user mode
	void startUserTime(pcb_t *p);

#endif