#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

    #include "scheduler.h"
    #include "utils.h"
    #include "const_bikaya.h"

    #ifdef TARGET_UMPS

        #include <umps/types.h>
        #include <umps/arch.h>
        #define INT_OLDAREA 0x20000000
    
    #endif

    #ifdef TARGET_UARM

        #include <uarm/uARMtypes.h>
        #include <uarm/uARMconst.h>
        #include <uarm/arch.h>

    #endif

    /* Interrupting devices bitmaps starting address: the actual bitmap address is computed with INT_INTBITMAP_START + (WORD_SIZE * (int_no - 3)) */
    #define PENDING_BITMAP_START 0x1000003c
    /* Physical memory frame size */
    #define WORD_SIZE 4
    /* funzione per ottenere il bitmap corrente della linea di interrupt */
    #define INTR_CURRENT_BITMAP(LINENO)  (unsigned int *)(PENDING_BITMAP_START + (WORD_SIZE * (LINENO - 3)))


    extern struct pcb_t *ACTIVE_PCB;
    extern struct pcb_t *GOODMORNING_PCB;
    #define TIME_SLICE 3000
    #define TERMSTATMASK 0xFF
    #define CMD_ACK          1

	void InterruptIntervalTimer();
    void InterruptTape();
    void InterruptTerminal();
    
#endif