#include "include/pcb.h"

HIDDEN LIST_HEAD(pcbFree_h);            //Macro di listx.h: inizializza la sentinella alla pcbFree. pcbFree_h è la sentinella della coda di pcb
HIDDEN struct pcb_t pcbFree_table[MAXPROC];	//Array statico contenente MAX_PROC pcb_t

/************************************/
/* PCB free list handling functions */
/************************************/

/* 1 -	Inizializza la pcbFree in modo da contenere tutti gli elementi della pcbFree_table.
		Questo metodo deve essere chiamato una volta sola in fase di inizializzazione della struttura dati. */
void initPcbs(void){
	for (int i=0; i<MAXPROC; i++){
		struct pcb_t* newPcb = &pcbFree_table[i];		//Mette in newPcp l'iesimo elemento della tabella dei pcb
		list_add_tail(&newPcb->p_next, &pcbFree_h); 	//Aggiunge l'elemento in coda alla lista pcbFree
	}
}

/* 2 -	Inserisce il PCB puntato da p nella lista dei PCB liberi (pcbFree) */
void freePcb(struct pcb_t *p){
	list_add_tail(&p->p_next, &pcbFree_h);
}

/* 3 - 	Restituisce NULL se la pcbFree è vuota. Altrimenti rimuove un elemento dalla pcbFree,
		inizializza tutti i campi (NULL/0) e restituisce l’elemento rimosso. */
struct pcb_t *allocPcb(void){

	//caso 1: pcbFree è vuota, return NULL
	if (list_empty(&pcbFree_h)) return NULL;

	struct pcb_t* tempPcb = container_of(pcbFree_h.prev, struct pcb_t, p_next);	//Restituisce puntatore all'ultimo elemento della pcbFree

	list_del(&tempPcb->p_next);					//Rimuove tempPcb dalla lista dei pcbFree: NON viene deallocato!

	//Inizializzazione di tutti i campi di tempPcb
	//Campi di tipo puntatore: imposto a NULL
	tempPcb->p_parent = NULL;
	tempPcb->p_semkey = NULL;

	//Campi di tipo intero: imposto a 0
	#ifdef TARGET_UMPS
	tempPcb->priority = 0;
	tempPcb->p_s.entry_hi = 0;
	tempPcb->p_s.cause = 0;
	tempPcb->p_s.status = 0;
	tempPcb->p_s.pc_epc = 0;
	tempPcb->p_s.hi = 0;
	tempPcb->p_s.lo = 0;
	tempPcb->user_start=0;
    tempPcb->user_total=0;
    tempPcb->kernel_start=0;
    tempPcb->kernel_total=0;
    tempPcb->start_time=0;
    tempPcb->wallclock_start=0;

	tempPcb->command=0;
	#endif

	#ifdef TARGET_UARM
	tempPcb->p_s.a1 = 0;  
	tempPcb->p_s.a2 = 0;   
	tempPcb->p_s.a3 = 0;  
	tempPcb->p_s.a4 = 0;   
	tempPcb->p_s.v1 = 0;   
	tempPcb->p_s.v2 = 0;    
	tempPcb->p_s.v3 = 0;  
	tempPcb->p_s.v4 = 0;   
	tempPcb->p_s.v5 = 0;   
	tempPcb->p_s.v6 = 0; 
	tempPcb->p_s.sl = 0;  
	tempPcb->p_s.fp = 0;    
	tempPcb->p_s.ip = 0;   
	tempPcb->p_s.sp = 0;    
	tempPcb->p_s.lr = 0;    
	tempPcb->p_s.pc = 0;    
	tempPcb->p_s.cpsr = 0;
	tempPcb->p_s.CP15_Control = 0;
	tempPcb->p_s.CP15_EntryHi = 0;
	tempPcb->p_s.CP15_Cause = 0;
	tempPcb->p_s.TOD_Hi = 0;
	tempPcb->p_s.TOD_Low = 0;
	tempPcb->user_start=0;
	tempPcb->user_total=0;
    tempPcb->kernel_start=0;
    tempPcb->kernel_total=0;
    tempPcb->start_time=0;
    tempPcb->wallclock_start=0;
	#endif

	//Campi di tipo list_head: uso INIT_LIST_HEAD
	INIT_LIST_HEAD(&tempPcb->p_next);
	INIT_LIST_HEAD(&tempPcb->p_child);
	INIT_LIST_HEAD(&tempPcb->p_sib);

	return tempPcb;
}

/********************************/
/* PCB queue handling functions */
/********************************/

/* 4 - 	Inizializza la lista dei PCB, inizializzando l’elemento sentinella. */
void mkEmptyProcQ(struct list_head *head){
	//Abbiamo già il puntatore a list_head, quindi usiamo INIT_LIST_HEAD
	INIT_LIST_HEAD(head);
}

/* 5 - 	Restituisce TRUE se la lista puntata da head è vuota, FALSE altrimenti. */
int emptyProcQ(struct list_head *head){
	return list_empty(head);
}

/* 6 - 	Inserisce l’elemento puntato da p nella coda dei processi puntata da head.
	L’inserimento deve avvenire tenendo conto della priorita’ di ciascun pcb (campo p->priority).
	La coda dei processi deve essere ordinata in base alla priorita’ dei PCB, in ordine decrescente
	(i.e. l’elemento di testa è l’elemento con la priorita’ più alta). */
void insertProcQ(struct list_head *head, struct pcb_t *p){
	struct pcb_t* i; //lo uso nel ciclo
	list_for_each_entry(i, head, p_next){
		if (p->priority > i->priority) {
			list_add(&p->p_next, list_prev(&i->p_next)); //Se la priorità di p è maggiore di quella di i, p va inserito tra il precedente di i e i.
			return; //Se arrivo qui, l'elemento l'ho già inserito -> Esco dalla funzione
		}
	}
	//La lista è vuota o p->priority è più basso di qualsiasi pcb presente nella lista.
	//Inserisco p in coda alla lista head
	list_add_tail(&p->p_next, head);
	return;
}

/* 7 - 	Restituisce l’elemento di testa della coda dei processi da head, SENZA RIMUOVERLO.
	Ritorna NULL se la coda non ha elementi. */
struct pcb_t *headProcQ(struct list_head *head){
	if (list_empty(head))
		return NULL;	//Nella lista c'è almeno un elemento: restituisco il primo (priorità massima)

	else{	//Restituisce puntatore al primo elemento della pcbFree (ma non lo rimuove!)
		struct pcb_t* tempPcb = container_of( list_next(head) , struct pcb_t, p_next);
		return tempPcb;
	}
}

/* 8 - 	Rimuove il primo elemento dalla coda dei processi puntata da head. Ritorna NULL se la coda è vuota.
	Altrimenti ritorna il puntatore all’elemento rimosso dalla lista. */
struct pcb_t *removeProcQ(struct list_head *head){
	if (list_empty(head))
		return NULL;
	//Nella lista c'è almeno un elemento: rimuovo e restituisco il primo (priorità massima)
	struct pcb_t* tempPcb = container_of(head->next, struct pcb_t, p_next); //Restituisce puntatore al primo elemento della  (non lo rimuove!)
	list_del(&tempPcb->p_next); //Rimuove tempPcb dalla lista head (NON lo dealloca!)
	return tempPcb;
}

/* 9 - 	Rimuove il PCB puntato da p dalla coda dei processi puntata da head. Se p non è presente nella coda,
	restituisce NULL. (NOTA: p può trovarsi in una posizione arbitraria della coda). */
struct pcb_t *outProcQ(struct list_head *head,struct pcb_t* p){
	struct pcb_t* i;
	list_for_each_entry(i, head, p_next){	//Nella lista head cerco un elemento uguale a p
		if(i==p){
			list_del(&i->p_next);	//Trovato: rimuove i dalla coda
			return i;
		}
	}
	return NULL;
}


/***********************/
/* Tree view functions */
/***********************/

/* 10 -	restituisce TRUE se il PCB puntato da p non ha figli, restituisce FALSE altrimenti. */
int emptyChild(struct pcb_t *this){
	return list_empty(&this->p_child); //TRUE se p_child è vuota, FALSE altrimenti
}

/* 11 -	Inserisce il PCB puntato da p come figlio del PCB puntato da prnt. */
void insertChild(struct pcb_t *prnt, struct pcb_t *p){
	list_add_tail(&p->p_sib, &prnt->p_child); 	//Inserisce p nella lista dei figli di prnt, 
							//ovvero inserisce p->p_sib alla lista che ha come sentinella prnt->p_child
	p->p_parent = prnt; //Aggiorna il campo p_parent di p a prnt.
}

/* 12 - Rimuove il primo figlio del PCB puntato da p. Se p non ha figli, restituisce NULL. */
struct pcb_t *removeChild(struct pcb_t *p){
	if(emptyChild(p)) return NULL;	//Se non ha figli -> NULL

	//p ha figli: restituisco il primo
	struct pcb_t *primofiglio = container_of(list_next(&p->p_child),struct pcb_t, p_sib); //pcb del primo figlio
	list_del(&(primofiglio->p_sib)); //Rimuove primofiglio dalla lista puntata da p_child: ovvero elimina p_sib dalla lista in cui è.
	primofiglio->p_parent = NULL; //Aggiorna a NULL il campo p_parent di primofiglio
	return primofiglio; //Restituisce il puntatore al primo figlio di p.
}

/* 13 - Rimuove il PCB puntato da p dalla lista dei figli del padre. Se il PCB puntato da p non ha un padre,
		restituisce NULL. Altrimenti restituisce l’elemento rimosso (cioè p). A differenza della removeChild,
		p può trovarsi in una posizione arbitraria (ossia non è necessariamente il primo figlio del padre). */
struct pcb_t *outChild(struct pcb_t *p){
	if(p->p_parent == NULL) return NULL; //Se p non ha un padre -> NULL

	//p è sicuramente nella lista dei figli del padre: lo rimuovo
	list_del(&p->p_sib); 
	p->p_parent = NULL; //p è diventato orfano: imposto a NULL il campo p_parent
	return p; //Restituisco l'elemento rimosso
}
