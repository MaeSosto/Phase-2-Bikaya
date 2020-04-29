#ifndef PCB_H
#define PCB_H

	#include "pcb.h"
	#include <const.h>
	#include "listx.h"
	#include "types_bikaya.h"

	/* PCB afree list handling functions 1 - 3 */
	void initPcbs(void);
	void freePcb(struct pcb_t *p);
	struct pcb_t *allocPcb(void);

	/* PCB queue handling functions 4 - 9*/
	void mkEmptyProcQ(struct list_head *head);
	int emptyProcQ(struct list_head *head);
	void insertProcQ(struct list_head *head, struct pcb_t *p);
	struct pcb_t *headProcQ(struct list_head *head);

	struct pcb_t *removeProcQ(struct list_head *head);
	struct pcb_t *outProcQ(struct list_head *head, struct pcb_t *p);


	/* PCB Tree view functions 10 - 13 */
	int emptyChild(struct pcb_t *this);
	void insertChild(struct pcb_t *prnt, struct pcb_t *p);
	struct pcb_t *removeChild(struct pcb_t *p);
	struct pcb_t *outChild(struct pcb_t *p);

#endif
