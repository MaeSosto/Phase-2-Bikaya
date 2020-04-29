#ifndef ASL_H
#define ASL_H

    #include "const.h"
    #include "pcb.h"
    #include <types_bikaya.h>

    /* ASL handling functions 14 - 20 */

    /* 14 - Restituisce il puntatore al SEMD nella ASL la cui chiave è pari a key. Se non esiste un elemento nella ASL con chiave eguale a key, viene restituito NULL.*/
    semd_t* getSemd(int *key); 

    /* 15 -	Viene inserito il PCB puntato da p nella coda dei processi bloccati associata al SEMD con chiave key. Se il semaforo corrispondente non è presente nella ASL, alloca un nuovo SEMD dalla lista di quelli liberi (semdFree) e lo inserisce nella ASL, settando i campi in maniera opportuna (i.e. key e s_procQ). Se non è possibile allocare un nuovo SEMD perché la lista di quelli liberi è vuota, restituisce TRUE. In tutti gli altri casi, restituisce FALSE. */
    int insertBlocked(int *key,pcb_t*p);

    /* 16 -	Ritorna il primo PCB dalla coda dei processi bloccati (s_ProcQ) associata al SEMD della ASL con chiave key. Se tale descrittore non esiste nella ASL, restituisce NULL. Altrimenti, restituisce l’elemento rimosso. Se la coda dei processi bloccati per il semaforo diventa vuota, rimuove il descrittore corrispondente dalla ASL e lo inserisce nella coda dei descrittori liberi (semdFree). */
    pcb_t* removeBlocked(int *key);

    /* 17 -	Rimuove il PCB puntato da p dalla coda del semaforo su cui è bloccato (indicato da p->p_semKey). Se il PCB non compare in tale coda, allora restituisce NULL (condizione di errore). Altrimenti, restituisce p. Se la coda dei processi bloccati per il semaforo diventa vuota, rimuove il descrittore corrispondente dalla ASL e lo inserisce nella coda dei descrittori liberi (semdFree).*/
    pcb_t* outBlocked(pcb_t *p);

    /* 18 -	Restituisce (senza rimuovere) il puntatore al PCB che si trova in testa alla coda dei processi associata al SEMD con chiave key. Ritorna NULL se il SEMD non compare nella ASL oppure se compare ma la sua coda dei processi è vuota. */
    pcb_t* headBlocked(int *key);

    /* 19 - Rimuove il PCB puntato da p dalla coda del semaforo su cui è bloccato (indicato da p->p_semKey). Inoltre, elimina tutti i processi dell’albero radicato in p (ossia tutti i processi che hanno come avo p) dalle eventuali code dei semafori su cui sono bloccati.*/
    void outChildBlocked(pcb_t *p);

    /* 20 -	Inizializza la lista dei semdFree in modo da contenere tutti gli elementi della semdTable. Questo metodo viene invocato una volta sola durante l’inizializzazione della struttura dati. */
    void initASL();

#endif
