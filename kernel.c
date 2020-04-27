#include "include/pcb.h"
#include "include/utils.h"
#include "include/scheduler.h"
#include "include/types_bikaya.h"
#include "include/asl.h"
#include "include/const_bikaya.h"
#include "include/syscall.h"

#ifdef TARGET_UMPS

	#include <umps/cp0.h>
	#include <umps/regdef.h>
	#include <umps/libumps.h>
	#include <umps/arch.h>
	#define MAX_DEVICES (DEV_USED_INTS * DEV_PER_INT) + DEV_PER_INT + 1

#endif

#ifdef TARGET_UARM

	#include <uarm/libuarm.h>
	#include <uarm/uARMconst.h>
	#include <uarm/arch.h>

#endif

extern void test();

LIST_HEAD(r_queue);
LIST_HEAD(asl_queue);
struct list_head* ready_queue = &(r_queue);
struct device_semd Semaforo;
int SemMem[MAX_DEVICES];

//MAIN
int main(){

    //Setto le Areas
    setAreas();

	//Inizializzo i semafori dei device
	InitSemd();

	//Inizializziamo la pcbFree
	initPcbs();
	initASL();

	//Instanziare il PCB e lo stato del singolo processo di test
	insertProcQ(ready_queue, initAllPCB((unsigned int) test, 1));

	#ifdef TARGET_UMPS
	
		//Setto lo status
		setSTATUS(getSTATUS() | STATUS_IEc | STATUS_IEp | STATUS_IM(2));
	
	#endif


	//Faccio partite lo scheduling
	Scheduling();

	return 0;

}