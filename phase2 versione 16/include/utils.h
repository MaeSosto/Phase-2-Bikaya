#ifndef _UTILS_H_
#define _UTILS_H_

	#include "types_bikaya.h"

	#ifdef TARGET_UMPS

		#include <umps/types.h>

	#endif

	#ifdef TARGET_UARM

		#include <uarm/uARMtypes.h>

	#endif

	//Inizializzo le Areas
	void setAreas();

	//Inizializzo i Pcb
	struct pcb_t *initAllPCB(unsigned int functionAddress, int priority);

	//Salvo lo stato della interrupt old area nel processo appena eseguito
	void SaveOldState(state_t* oldarea, state_t* processo);

	//Salva il processo prima di poterlo mettere di nuovo nella ready queue
	void SaveProc();

	//Alloca spazio in memoria per i semafori dei device
	void InitSemd();

	//Qualsisi sia l'indirizzo passato (di una qualsiasi linea), restituisce un numero da 0 a 7 (quale device)
	int numDev(unsigned int *registro);

	//Qualsiasi sia l'indirizzo passato, restituisce un numero da 3 a 7 (quale linea)
	int numLine(unsigned int *registro);
	
	void stampaInt(int n);
#endif