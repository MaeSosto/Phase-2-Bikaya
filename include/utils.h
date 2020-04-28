#ifndef _UTILS_H_
#define _UTILS_H_

  #include "types_bikaya.h"

  #ifdef TARGET_UMPS

    #include <umps/types.h>

  #endif

  #ifdef TARGET_UARM

    #include <uarm/uARMtypes.h>

  #endif

  void setAreas();
  struct pcb_t *initAllPCB(unsigned int functionAddress, int priority);
  void SaveOldState(state_t* oldarea, state_t* processo);
  
#endif