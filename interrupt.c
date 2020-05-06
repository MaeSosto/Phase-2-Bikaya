#include "include/interrupt.h"

/* Interrupting devices bitmaps starting address: the actual bitmap address is
   computed with INT_INTBITMAP_START + (WORD_SIZE * (int_no - 3)) */
#define PENDING_BITMAP_START 0x1000003c

/* Physical memory frame size */
#define WORD_SIZE 4

/* funzione per ottenere il bitmap corrente della linea di interrupt */
#define INTR_CURRENT_BITMAP(LINENO)  (unsigned int *)(PENDING_BITMAP_START + (WORD_SIZE * (LINENO - 3)))


void teminal_rec(){}

void bp_interrupt_terminal(){}
void bp_interrupt_timer(){}
void bp_disk(){}
void bp_tape(){}
void bp_network(){}
void bp_printer(){}

extern int tempo;

void InterruptIntervalTimer(){

	bp_interrupt_timer();

	//Setto il timer (ACK)
  	*(unsigned int*)BUS_REG_TIMER = TIME_SLICE;

	tempo = TRUE;

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


//TAPE 4
void InterruptTape(){

	bp_tape();

	//termprint ("Interrupt: Gestisco l'int del tape ");
	//Per tutti i device di questa linea 
	for(int i = 0; i < DEV_PER_INT; i++){

		//Controllo se l'interrupt è stato lanciato da questa linea
		if( ((CDEV_BITMAP_ADDR(INT_TAPE))) == i){

			//termprint("InterruptTape: gestisco il dispositivo \n ");
			stampaInt(i);
			termprint("\n");
			
			//Prendo il registro del device che ha lanciato l'interrupt
			dtpreg_t *reg = (dtpreg_t *)DEV_REG_ADDR(INT_TAPE, i);

			//Faccio la verhogen = sblocco il processo, lo sveglio
			Verhogen(Semaforo.tape[i].s_key);
			GOODMORNING_PCB->p_s.reg_v0 = reg->status;

						
			
			
			//termprint("InterruptTape: Faccio ACK \n ");
			//Faccio ACK
			reg->command = CMD_ACK;

		}

	}

}

//TERMINAL 7
void InterruptTerminal(){

	bp_interrupt_terminal();

	//Per tutti i device di questa linea 
	for(int i = 0; i < DEV_PER_INT; i++){

		//Controllo se l'interrupt è stato lanciato da questa linea

		if(getDevice(7,i)){
			
			//Prendo il registro del device che ha lanciato l'interrupt
			termreg_t *reg = (termreg_t *)DEV_REG_ADDR(INT_TERMINAL, i);
			
			//QUA LO STATUS È CHARACTER RECEIVED = 5

			//Controllo se il terminale è in trasmissione o ricezione

			//Il terminale in RICEZIONE non è ready
			if((reg->recv_status & TERMSTATMASK)!= 1){

				if(*Semaforo.terminalR[i].s_key < 0){
									
					//Sblocco il processo sul terminale in ricezione del device richiesto
					Verhogen(Semaforo.terminalR[i].s_key);

					//Aggiorno lo status del processo svegliato
					GOODMORNING_PCB->p_s.reg_v0 = reg->recv_status;
									
				}
				
				else{

					//Invio ACK
					reg->recv_command = 1;
				
				}
				
			}	

			//Il terminale in TRASMISSIONE non è ready
			if((reg->transm_status & TERMSTATMASK)!= 1){

				if(*Semaforo.terminalT[i].s_key < 0){
									
					//Sblocco il processo sul terminale in ricezione del device richiesto
					Verhogen(Semaforo.terminalT[i].s_key);

					//Aggiorno lo status del processo svegliato
					GOODMORNING_PCB->p_s.reg_v0 = reg->transm_status;
									
				}
				
				else{

					//Invio ACK
					reg->transm_command = 1;
				
				}
				
			}	

		}
	
	}

}

//line_no ->7 terminale DEVICE - DA 3 A 7
//dev_no -> DA 0 A 8
int getDevice(int line_no, int dev_no){

  if(*INTR_CURRENT_BITMAP(line_no) & (1 << dev_no))
    return 1;


  return 0;
}