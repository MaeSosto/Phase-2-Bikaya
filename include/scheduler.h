#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

	#include "pcb.h"
	#include "listx.h"
	#include "types_bikaya.h"
	#include "utils.h"

	#ifdef TARGET_UMPS

		#include <umps/libumps.h>
		#include <umps/arch.h>
		#include <umps/cp0.h>

		#define BUS_TODLOW  0x1000001c
		#define BUS_TODHIGH 0x10000018
		#define getTODLO() (*((unsigned int *)BUS_TODLOW))

	#endif

	#ifdef  TARGET_UARM

		#include <uarm/arch.h>
		#include <uarm/libuarm.h>
		
	#endif

	#define TIME_SLICE 3000

	extern struct list_head* ready_queue;
	extern struct pcb_t *ACTIVE_PCB;
	extern unsigned int BLOCK_COUNT;
	extern void stampaInt(int n);

	void Scheduling();

#endif