#include "include/pcb.h"
#include "include/scheduler.h"
#include "include/utils.h"
#include "include/types_bikaya.h"
#include "include/asl.h"
#include "include/const_bikaya.h"
#include "include/syscall.h"

#ifdef TARGET_UMPS

	#include <umps/cp0.h>
	#include <umps/regdef.h>
	#include <umps/libumps.h>
	#include <umps/arch.h>

#endif

#ifdef TARGET_UARM

	#include <uarm/libuarm.h>
	#include <uarm/uARMconst.h>
	#include <uarm/arch.h>

#endif

extern void test();

#define MAX_SEM 48
LIST_HEAD(r_queue);
LIST_HEAD(asl_queue);
struct list_head* ready_queue = &(r_queue);
struct device_semd Semaforo;
int SemMem[MAX_SEM];
struct pcb_t *ACTIVE_PCB = NULL;
struct pcb_t *GOODMORNING_PCB = NULL;
//Viene incrementato/decrementato quando si aggiungono/rimuovono processi bloccati
unsigned int BLOCK_COUNT = 0;

//MAIN
int main(){

    //Setto le Areas
    setAreas();

	//Inizializzo i semafori dei device
	InitSemd();

	//Inizializziamo la pcbFree
	initPcbs();
	
	//Inizializza la lista dei semdFree
	initASL();

	//Instanziare il PCB e lo stato del singolo processo di test
	insertProcQ(ready_queue, initAllPCB((unsigned int) test, 1));

	//Faccio partite lo scheduling
	Scheduling();

	return 0;

}