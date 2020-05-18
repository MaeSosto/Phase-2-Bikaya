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
            
        //Ritorna 1 se l'int_no (interrupt number) è in attesa
        #define CAUSE_IP_GET(cause, int_no) ((cause) & (1 << ((int_no) + 8)))

    #endif

    #ifdef TARGET_UARM

        #include <uarm/uARMconst.h>
        #include <uarm/uARMtypes.h>
        #include <uarm/arch.h>
        #include <uarm/libuarm.h>

        //Ritorna il valore dell'eccezione che si è sollevata nel campo status
        #define CAUSE_EXCCODE_MASK     0x0000007c
        #define CAUSE_EXCCODE_BIT      2
        #define CAUSE_GET_EXCCODE(x)   (((x) & CAUSE_EXCCODE_MASK) >> CAUSE_EXCCODE_BIT)

        #define EXC_BP EXC_BREAKPOINT

    #endif

    #define INTERRUPT_PENDING_MASK     	0x0000ff00
    #define INTERRUPT_PENDING_B      	8
    #define INTERRUPT_PENDING_FUNC(x)   (((x) & INTERRUPT_PENDING_MASK) >> INTERRUPT_PENDING_B)

    #define INT_IP_GET(cause) ((cause >> 8) & 0xFF)
        
    extern struct pcb_t *ACTIVE_PCB;
    extern struct list_head* ready_queue;

    //Dichiarazione delle funzioni
    void syscallHandler();
    void trapHandler();
    void tlbHandler();
    void interruptHandler();
    
#endif