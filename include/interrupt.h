#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

    #include "include/scheduler.h"
    #include "include/utils.h"
    #include "include/const_bikaya.h"

    #ifdef TARGET_UMPS

        #include <umps/types.h>
        #define INT_OLDAREA 0x20000000
    
    #endif

    #ifdef TARGET_UARM

        #include <uarm/uARMtypes.h>
        #include <uarm/uARMconst.h>

    #endif

    extern struct pcb_t *ACTIVE_PCB;

	void interruptHandler();
    
#endif