#include "include/interrupt.h"

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
		if(Eccezione(7,i)){
			
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

