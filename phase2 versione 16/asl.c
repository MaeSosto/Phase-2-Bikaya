/****************************************/
/* Progetto  SO 2019 - Rikaya      	*/
/* Gruppo lso19az18                 	*/
/* Ultimo agg: 24/12/2018 		*/
/****************************************/

#include "include/asl.h"
#include "include/const.h"
#include "include/pcb.h"

HIDDEN LIST_HEAD(semd_h); 		//semd_h è la sentinella della lista ASL (lista dei semd attivi)

HIDDEN LIST_HEAD(semdFree_h);		//semdFree_h è la sentinella della lista semdFree (lista dei semd liberi)

/**************************/
/* ASL handling functions */
/**************************/

/* 14 - Restituisce il puntatore al SEMD nella ASL la cui chiave è pari a key.
		Se non esiste un elemento nella ASL con chiave eguale a key, viene restituito NULL.*/
semd_t* getSemd(int *key){

	if (list_empty(&semd_h)) return NULL; 		//se ASL è vuota -> NULL
	semd_t* i; 	//"indice" del ciclo
	list_for_each_entry(i, &semd_h, s_next){	//cicla sulla lista ASL partendo dalla sentinella semd_h
		if(key == (i->s_key)) return i;		//se trova un semd in asl, lo restituisce
	}
	return NULL;	//non c'è un semd in asl con quella key -> restituisce NULL
}

/* 15 -	Viene inserito il PCB puntato da p nella coda dei processi bloccati associata
        al SEMD con chiave key. Se il semaforo corrispondente non è presente nella ASL,
        alloca un nuovo SEMD dalla lista di quelli liberi (semdFree) e lo inserisce
        nella ASL, settando i campi in maniera opportuna (i.e. key e s_procQ). Se non 
        è possibile allocare un nuovo SEMD perché la lista di quelli liberi è vuota,
        restituisce TRUE. In tutti gli altri casi, restituisce FALSE. */
int insertBlocked(int *key, pcb_t* p){
	semd_t* semd =  getSemd(key);

	//caso 1: Esiste SEMD corrispondente alla key:
	if (semd != NULL){
		insertProcQ(&semd->s_procQ , p);	//aggiunge il PCB nella lista in base alla sua priorità
		p->p_semkey=key;
		return FALSE;
	}

	//caso 2: SEMD non presente nella ASL && lista semdFree è vuota
	if(list_empty(&semdFree_h)) return TRUE;

	//caso 3: SEMD non presente nella ASL && esiste un SEMD libero
	semd_t *new_semd = container_of(semdFree_h.next , semd_t, s_next);
	list_del(&new_semd->s_next);			//elimina new_semd dalla lista semdFree
	list_add_tail(&new_semd->s_next, &semd_h);	//aggiunge semd alla coda ASL
	new_semd->s_key = key;				//assegna la key al semd
	mkEmptyProcQ(&new_semd->s_procQ);		//crea lista vuota di pcb puntata da s_procQ
	insertProcQ(&new_semd->s_procQ, p);		//inseririsce il SEMD nella coda di p
	p->p_semkey=key;				//aggiorna la semkey di p
	return FALSE;
}

/* 16 -	Ritorna il primo PCB dalla coda dei processi bloccati (s_ProcQ) associata al SEMD 
		della ASL con chiave key. Se tale descrittore non esiste nella ASL, restituisce NULL.
		Altrimenti, restituisce l’elemento rimosso. Se la coda dei processi bloccati per
		il semaforo diventa vuota, rimuove il descrittore corrispondente dalla ASL e lo 
		inserisce nella coda dei descrittori liberi (semdFree). */
pcb_t* removeBlocked(int *key){
	semd_t* semd = getSemd(key);

	//caso 1: SEMD non esiste nella ASL
	if(semd == NULL) return NULL;

	//caso 2: ritorna il primo PCB dalla coda dei processi bloccati 
	pcb_t* pcb_tmp = removeProcQ(&semd->s_procQ);	//rimuove il primo pcb dalla coda

	//controlli per vedere se eliminare semd da ASL
	if(list_empty(&semd->s_procQ)) {
		list_del(&semd->s_next);		//è vuota => elimina SEMD da ASL
		list_add(&semd->s_next, &semdFree_h);	//aggiunge semd alla lista semdFree (indifferente testa/coda)
	}
	return pcb_tmp;		//ritorna il puntatore al primo processo bloccato del semd
}

/* 17 -	Rimuove il PCB puntato da p dalla coda del semaforo su cui è bloccato (indicato da 
		p->p_semKey). Se il PCB non compare in tale coda, allora restituisce NULL (condizione di errore).
		Altrimenti, restituisce p. Se la coda dei processi bloccati per il semaforo diventa vuota, rimuove 
		il descrittore corrispondente dalla ASL e lo inserisce nella coda dei descrittori liberi (semdFree).*/

pcb_t* outBlocked(pcb_t *p){

	int *key = p->p_semkey;
	semd_t* semd = getSemd(key);	//prende il SEMD associato alla key
	pcb_t* pcb = outProcQ(&semd->s_procQ, p);	//elimina il PCB puntato da p, nella coda puntata da semd->s_procQ

	//controlli per vedere se eliminare semd da ASL
	if(list_empty(&semd->s_procQ)) {
		list_del(&semd->s_next);		//è vuota => elimina SEMD da ASL
		list_add(&semd->s_next, &semdFree_h);	//aggiunge semd alla lista semdFree (indifferente testa/coda)
	}

	return pcb;
}

/* 18 -	Restituisce (senza rimuovere) il puntatore al PCB che si trova in testa alla coda dei
		processi associata al SEMD con chiave key. Ritorna NULL se il SEMD non compare nella ASL oppure
		se compare ma la sua coda dei processi è vuota. */
pcb_t* headBlocked(int *key){
	semd_t* semd = getSemd(key);		//prende il SEMD associato alla key
	if(semd == NULL) return NULL;		//SEMD non compare nella ASL (NULL)
	pcb_t* pcb = headProcQ(&semd->s_procQ);
	return pcb;		//primo processo nella lista s_procQ (ritorna puntatore)
}

/* 19 - Rimuove il PCB puntato da p dalla coda del semaforo su cui è bloccato (indicato da
		p->p_semKey). Inoltre, elimina tutti i processi dell’albero radicato in p (ossia tutti
		i processi che hanno come avo p) dalle eventuali code dei semafori su cui sono bloccati.*/

void outChildBlocked(pcb_t *p){
	pcb_t * firstPcb = p;

	pcb_t * pcb = outBlocked(p);	//Toglie il pcb dalla coda del semd su cui è bloccato
	if (pcb == NULL) return;
	if (emptyChild(p)) return;		//Caso base: pcb non ha figli, quindi finisce

	else {	//Pcb ha dei figli quindi per ogni figlio richiamo la outChildBlocked ricorsivamente
		pcb_t * i;	//Elemento del ciclo
		list_for_each_entry(i, &pcb->p_child, p_sib) {
			outChildBlocked(i);	//ricorsione sull'elemento
		}
	}

	//controllo per vedere se eliminare semd da ASL
	int *key = firstPcb->p_semkey;
	semd_t* semd = getSemd(key);
	if(list_empty(&semd->s_procQ)) {
		list_del(&semd->s_next);		//è vuota => elimina SEMD da ASL
		list_add(&semd->s_next, &semdFree_h);	//aggiunge semd alla lista semdFree (indifferente testa/coda)
	}
}

/* 20 -	Inizializza la lista dei semdFree in modo da contenere tutti gli elementi della semdTable.
		Questo metodo viene invocato una volta sola durante l’inizializzazione della struttura dati. */
void initASL(void){

	HIDDEN semd_t semdFree_table[MAXPROC]; //Array statico contenente MAX_PROC semd_t

	for(int i = 0; i < MAXPROC; i++){
		semd_t* newSemd = &semdFree_table[i];		//Mette in newPcp l'iesimo elemento della tabella dei pcb
		list_add_tail(&newSemd->s_next, &semdFree_h);	//aggiungo l'i-esimo elemento della semdTable in coda
	}
}
