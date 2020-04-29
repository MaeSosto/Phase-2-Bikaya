#include "include/interrupt.h"

void InterruptIntervalTimer(){

	//Setto il timer (ACK)
  	*(unsigned int*)BUS_REG_TIMER = TIME_SLICE;


	//Ho già un processo attivo
	if(ACTIVE_PCB != NULL){

		//Tornare in user mode

		//Salvo i registri dell'old area dell'interrupt al processo
		struct state *AREA=(state_t *) INT_OLDAREA;

		/* Copio lo stato della old area dell'intertupt nel processo che lo ha sollevato */
		SaveOldState(AREA, &(ACTIVE_PCB->p_s));
		
	}

	//Non ho processi in esecuzione
	else{

		//Riparto con lo scheduler

	}

}


// NON NOSTRA
// int getDevice(int line_no, int dev_no){

//   /*questa funzione ritorna 1 se il device i attaccato alla linea j solleva un interrupt.
//   Per identificare il device i si sfrutta l'indirizzo di PENDING_BITMAP_START, ovvero
//   l'indirizzo in memoria dove inizia la Interrupting Devices Bit Map*/

//   /*una parola è riservata in memoria per indicare quale device ha interrupts pendenti
//   sulle linee da 3 a 7. Quindi se si tratta del terminale per esempio, è necessario spostarsi
//   di 7 - 3 = 4 parole in avanti da PENDING_BITMAP_START. Successivamente è necessario fare uno shift
//   di un 1 a sinistra di dev_no posizioni per vedere se il device dev_no associato a line_no ha un interrupt pendente.
//   Dopo lo shift si fa un & bitwise con *INTR_CURRENT_BITMAP(line_no) in modo da vedere se effettivamente il bit destinato a
//   dev_no è 1. */

//   if(*INTR_CURRENT_BITMAP(line_no) & (1 << dev_no))
//     return 1;


//   return 0;
// }


// /* Funzione per trovare quale dispositivo ha causato l'interrupt */
// HIDDEN inline int whichDevice(u32 bitmap) {
//     int dev_n = 0;
//     for(; dev_n<8; dev_n++ ){
//         if( bitmap && (1UL << dev_n ) )
//             break;
//     }
//     return dev_n;
// }



void InterruptTape(){

	termprint ("Interrupt: Gestisco l'int del tape ");
	//Per tutti i device di questa linea 
	for(int i = 0; i < DEV_PER_INT; i++){

		//Controllo se l'interrupt è stato lanciato da questa linea
		if( ((CDEV_BITMAP_ADDR(INT_TAPE))) == i){

			termprint("InterruptTape: gestisco il dispositivo \n ");
			stampaInt(i);
			termprint("\n");
			
			//Prendo il registro del device che ha lanciato l'interrupt
			dtpreg_t *reg = (dtpreg_t *)DEV_REG_ADDR(INT_TAPE, i);

			//Faccio la verhogen = sblocco il processo, lo sveglio
			Verhogen(Semaforo.tape[i].s_key);
			GOODMORNING_PCB->p_s.reg_v0 = reg->status;
			termprint("InterruptTape: Faccio ACK \n ");
			//Faccio ACK
			reg->command = CMD_ACK;

		}

	}

}



