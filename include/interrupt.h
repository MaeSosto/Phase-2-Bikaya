#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

    #include "scheduler.h"
    #include "utils.h"
    #include "const_bikaya.h"

    #ifdef TARGET_UMPS

        #include <umps/types.h>
        #include <umps/arch.h>
        
        #define INT_OLDAREA 0x20000000

        #define PENDING_BITMAP_START 0x1000003c

    #endif

    #ifdef TARGET_UARM

        #include <uarm/uARMtypes.h>
        #include <uarm/uARMconst.h>
        #include <uarm/arch.h>

        #define PENDING_BITMAP_START 0x00006FE0

    #endif
   
    //Grandezza di una word
    #define WORD_SIZE 4
    
    //Funzione per ottenere il bitmap corrente della linea di interrupt 
    #define INTR_CURRENT_BITMAP(LINENO)  (unsigned int *)(PENDING_BITMAP_START + (WORD_SIZE * (LINENO - 3)))

    #define TIME_SLICE 3000
    #define TERMSTATMASK 0xFF   
    #define CMD_ACK          1

    extern struct pcb_t *ACTIVE_PCB;
    extern struct pcb_t *GOODMORNING_PCB;
    
    //Dichiarazioni delle funzioni
    void InterruptPLC();
	void InterruptIntervalTimer();
    void InterruptDisk();
    void InterruptNetwork();
    void InterruptPrinter();
    void InterruptTape();
    void InterruptTerminal();
    
#endif