#ifndef _SYSCALL_H_
#define _SYSCALL_H_

    void getCPUTime(unsigned int *user, unsigned int *kernel, unsigned int *wallclock);

    //SYSCALL 2
    int CreateProcess(struct state *statep, int priority, void ** cpid);

    //SYSCALL 3
    int TerminateProcess(void * pid);
    
    //SYSCALL 4
    void Verhogen(int *semaddr);

    //SYSCALL 5
    void Passeren(int *semaddr);

    //SYSCALL 6
    int DO_IO(unsigned int command, unsigned int *registro, int subdevice);
    
    //SYSCALL 7
    int SpecPassup(int type, struct state *old, struct state *nuovo);

    //SYSCALL 8
    void getPid(void ** pid, void ** ppid);

#endif
