#ifndef _SYSCALL_H_
#define _SYSCALL_H_

    #include "const_bikaya.h"
    #include "types_bikaya.h"
    #include "listx.h"
    #include "pcb.h"
    #include "asl.h"
    #include "utils.h"

    #ifdef TARGET_UMPS

        #include <umps/libumps.h> 
        #include <umps/types.h>
        #include <umps/arch.h>

    #endif

    #ifdef TARGET_UARM

        #include <uarm/arch.h>
        #include <uarm/uARMtypes.h>
        #include <uarm/uARMconst.h>

    #endif
    
    #define MAX_SEM 48
    #define BUS_TODLOW  0x1000001c
    #define BUS_TODHIGH 0x10000018
    #define getTODLO() (*((unsigned int *)BUS_TODLOW))

    extern struct pcb_t *ACTIVE_PCB;
    extern struct pcb_t *GOODMORNING_PCB;
    extern unsigned int BLOCK_COUNT;
    extern struct list_head* ready_queue;
    extern struct device_semd Semaforo;
    extern int SemMem[MAX_SEM];
    extern void termprint(char *str);  

    //SYSCALL 1
    void getCPUTime(unsigned int *user, unsigned int *kernel, unsigned int *wallclock);

    //SYSCALL 2
    int CreateProcess(state_t *statep, int priority, void ** cpid);

    //SYSCALL 3
    int TerminateProcess(void * pid);
    
    //SYSCALL 4
    void Verhogen(int *semaddr);

    //SYSCALL 5
    void Passeren(int *semaddr);

    //SYSCALL 6
    int DO_IO(unsigned int command, unsigned int *registro, int subdevice);
    
    //SYSCALL 7
    int SpecPassup(int type, state_t *old, state_t *nuovo);

    //SYSCALL 8
    void getPid(void ** pid, void ** ppid);

#endif
