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

    //locazione in memoria dell'Interrupting Devices Bit Map
    #define INTERRUPTING_DEVICES_BIT_MAP 0x10000003C
    //macro per recuperare il registro della linea su cui è pendente un interrupt
    #define DEVICE_REGISTER(line, dev)  ( 0x1000.0050 + ((line - 3) * 0x80) + (device * 0x10) )

    extern struct pcb_t *ACTIVE_PCB;
    extern struct pcb_t *GOODMORNING_PCB;
    #define TIME_SLICE 3000

    #define CMD_ACK          1

	void InterruptIntervalTimer();
    void InterruptTape();
    
#endif