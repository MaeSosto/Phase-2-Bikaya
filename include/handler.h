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
            
        /* Returns 1 if the interrupt int_no is pending */
        #define CAUSE_IP_GET(cause, int_no) ((cause) & (1 << ((int_no) + 8)))
        
        extern void termprint(char* str);

    #endif

    #ifdef TARGET_UARM

        #include <uarm/uARMconst.h>
        #include <uarm/uARMtypes.h>
        #include <uarm/arch.h>
        #include <uarm/libuarm.h>

        /*
        * CP0 Cause fields
        */
        #define CAUSE_EXCCODE_MASK     0x0000007c
        #define CAUSE_EXCCODE_BIT      2
        #define CAUSE_GET_EXCCODE(x)   (((x) & CAUSE_EXCCODE_MASK) >> CAUSE_EXCCODE_BIT)

        #define EXC_BP EXC_BREAKPOINT

        //PER UARM VEDI FILE UARMcost.h riga 164
        //#define CAUSE_IP_GET(cause, int_no) ((cause) & (1 << ((int_no) + 24)))

    #endif

    #define BUS_TODLOW  0x1000001c
    #define BUS_TODHIGH 0x10000018
    #define getTODLO() (*((unsigned int *)BUS_TODLOW))
        
    extern struct pcb_t *ACTIVE_PCB;
    extern struct list_head* ready_queue;

    //dichiarazione delle funzioni
    void syscallHandler();
    void trapHandler();
    void tlbHandler();
    void interruptHandler();
    
#endif