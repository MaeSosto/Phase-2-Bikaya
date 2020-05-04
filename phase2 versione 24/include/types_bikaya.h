#ifndef _TYPES11_H
#define _TYPES11_H

    #include "const.h"
    #include "listx.h"

    #ifdef TARGET_UMPS

        #include <umps/types.h>

    #endif

    #ifdef TARGET_UARM

        #define UARM_MACHINE_COMPILING
        #include <uarm/uARMtypes.h>

    #endif



    typedef unsigned int memaddr;

    // Process Control Block (PCB) data structure
    typedef struct pcb_t {
        /*process queue fields */
        struct list_head p_next;

        /*process tree fields */
        struct pcb_t *   p_parent;
        struct list_head p_child, p_sib;

        /* processor state, etc */
        state_t p_s;

        /* dynamic process priority */
        int priority;
        /* initial process priority */
        int original_priority;

        /* key of the semaphore on which the process is eventually blocked */
        int *p_semkey;

        // campo command del pcb
        int command;


        // timer di esecuzione
        unsigned int user_start;
        unsigned int user_total;
        unsigned int kernel_start;
        unsigned int kernel_total;
        unsigned int start_time;
        unsigned int wallclock_start;

    } pcb_t;



    // Semaphore Descriptor (SEMD) data structure
    typedef struct semd_t {
        struct list_head s_next;

        // Semaphore key
        int *s_key;

        // Queue of PCBs blocked on the semaphore
        struct list_head s_procQ;
    } semd_t;

    typedef struct device_semd {
        semd_t disk[DEV_PER_INT];
        semd_t tape[DEV_PER_INT];
        semd_t network[DEV_PER_INT];
        semd_t printer[DEV_PER_INT];
        semd_t terminalT[DEV_PER_INT];
        semd_t terminalR[DEV_PER_INT];
    } device_semd;


#endif