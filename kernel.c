#include "include/pcb.h"
#include "include/utils.h"
#include "include/scheduler.h"
#include "include/types_bikaya.h"

#ifdef TARGET_UMPS

	#include <umps/cp0.h>
	#include <umps/regdef.h>
	#include <umps/libumps.h>

#endif

#ifdef TARGET_UARM

	#include <uarm/libuarm.h>
	#include <uarm/uARMconst.h>
	#include <uarm/arch.h>

#endif

extern void test1();
extern void test2();
extern void test3();

LIST_HEAD(r_queue);
struct list_head* ready_queue = &(r_queue);

//MAIN
int main(){

    //Setto le Areas
    setAreas();

	//Inizializziamo la pcbFree
	initPcbs();
	
	//Inserisco i Pcb inizializzati nella Ready Queue
	insertProcQ(ready_queue, initAllPCB((unsigned int) test3, 3));
	insertProcQ(ready_queue, initAllPCB((unsigned int) test1, 1));
	insertProcQ(ready_queue, initAllPCB((unsigned int) test2, 2));

	
	#ifdef TARGET_UMPS
	
		//Setto lo status
		setSTATUS(getSTATUS() | STATUS_IEc | STATUS_IEp | STATUS_IM(2));
	
	#endif

	//Faccio partite lo scheduling
	Scheduling();

	return 0;

}