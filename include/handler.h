#ifndef _HANDLER_H_
#define _HANDLER_H_

    #include "scheduler.h"
    #include "pcb.h"
    #include "utils.h"
    #include "syscall.h"
    #include "const_bikaya.h"
    #include "interrupt.h"

    #ifdef TARGET_UMPS

        #include "umps/arch.h"
        #include "umps/cp0.h"
        #include <umps/libumps.h>
        #define SYSBK_OLDAREA 0x20000348
        
        /* Returns 1 if the interrupt int_no is pending */
        #define CAUSE_IP_GET(cause, int_no) ((cause) & (1 << ((int_no) + 8)))
        
        extern void termprint(char* str);

    #endif

    #ifdef TARGET_UARM

        #include <uarm/uARMconst.h>
        #include <uarm/uARMtypes.h>
        #include <uarm/arch.h>
        #include <uarm/libuarm.h>
        extern void tprint(char* str);

        //PER UARM VEDI FILE UARMcost.h riga 164
        //#define CAUSE_IP_GET(cause, int_no) ((cause) & (1 << ((int_no) + 24)))

    #endif

    #define BUS_TODLOW  0x1000001c
    #define BUS_TODHIGH 0x10000018
    #define getTODLO() (*((unsigned int *)BUS_TODLOW))
    
    //OLD AREAS
    #define TRAP_OLDAREA 0x20000230
    #define TLB_OLDAREA 0x20000118
    #define SYSCALL_OLDAREA 0x20000348
    
    extern struct pcb_t *ACTIVE_PCB;
    extern struct list_head* ready_queue;

    //dichiarazione delle funzioni
    void syscallHandler();
    void trapHandler();
    void tlbHandler();
    void interruptHandler();
    
#endif