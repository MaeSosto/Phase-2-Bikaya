#include "include/interrupt.h"

extern int tempo;

//PROCESSOR LOCAL TIMER
void InterruptPLC(){
	
	//????
	// 	 setTIMER(TIME_SLICE);
    //   scheduler();

}

//INTERVAL TIMER 2
void InterruptIntervalTimer(){

	//Setto il timer (ACK)
  	*(unsigned int*)BUS_REG_TIMER = TIME_SLICE;

	tempo = TRUE;

}

//DISK 3
void InterruptDisk(){

	//Per tutti i device di questa linea 
	for(int i = 0; i < DEV_PER_INT; i++){

		//Controllo se l'interrupt è stato lanciato da questa linea
		if(Eccezione(3, i)){
			
			//Prendo il registro del device che ha lanciato l'interrupt
			dtpreg_t *reg = (dtpreg_t *)DEV_REG_ADDR(INT_DISK, i);
			
			if(*Semaforo.disk[i].s_key < 0){
								
				//Sblocco il processo sul terminale in ricezione del device richiesto
				Verhogen(Semaforo.disk[i].s_key);

				#ifdef TARGET_UMPS
					
					//Aggiorno lo status del processo svegliato
					GOODMORNING_PCB->p_s.reg_v0 = reg->status;

				#endif

				#ifdef TARGET_UARM

					//Aggiorno lo status del processo svegliato
					GOODMORNING_PCB->p_s.a1 = reg->status;


				#endif
								
			}
			
			else{

				//Invio ACK
				reg->command = CMD_ACK;
			
			}

		}
	
	}

}

//TAPE 4
void InterruptTape(){

	//Per tutti i device di questa linea 
	for(int i = 0; i < DEV_PER_INT; i++){

		//Controllo se l'interrupt è stato lanciato da questa linea
		if(Eccezione(4, i)){
			
			//Prendo il registro del device che ha lanciato l'interrupt
			dtpreg_t *reg = (dtpreg_t *)DEV_REG_ADDR(INT_TAPE, i);
			
			if(*Semaforo.tape[i].s_key < 0){
								
				//Sblocco il processo sul terminale in ricezione del device richiesto
				Verhogen(Semaforo.tape[i].s_key);

				#ifdef TARGET_UMPS
					
					//Aggiorno lo status del processo svegliato
					GOODMORNING_PCB->p_s.reg_v0 = reg->status;

				#endif

				#ifdef TARGET_UARM

					//Aggiorno lo status del processo svegliato
					GOODMORNING_PCB->p_s.a1 = reg->status;


				#endif
								
			}
			
			else{

				//Invio ACK
				reg->command = CMD_ACK;
			
			}

		}
	
	}

}

//NETWORK 5
void InterruptNetwork(){

	//Per tutti i device di questa linea 
	for(int i = 0; i < DEV_PER_INT; i++){

		//Controllo se l'interrupt è stato lanciato da questa linea
		if(Eccezione(5, i)){
			
			//Prendo il registro del device che ha lanciato l'interrupt
			dtpreg_t *reg = (dtpreg_t *)DEV_REG_ADDR(INT_UNUSED, i);
			
			if(*Semaforo.network[i].s_key < 0){
								
				//Sblocco il processo sul terminale in ricezione del device richiesto
				Verhogen(Semaforo.network[i].s_key);

				#ifdef TARGET_UMPS
					
					//Aggiorno lo status del processo svegliato
					GOODMORNING_PCB->p_s.reg_v0 = reg->status;

				#endif

				#ifdef TARGET_UARM

					//Aggiorno lo status del processo svegliato
					GOODMORNING_PCB->p_s.a1 = reg->status;


				#endif
								
			}
			
			else{

				//Invio ACK
				reg->command = CMD_ACK;
			
			}

		}
	
	}

}

//PRINTER 6
void InterruptPrinter(){

	//Per tutti i device di questa linea 
	for(int i = 0; i < DEV_PER_INT; i++){

		//Controllo se l'interrupt è stato lanciato da questa linea
		if(Eccezione(6, i)){
			
			//Prendo il registro del device che ha lanciato l'interrupt
			dtpreg_t *reg = (dtpreg_t *)DEV_REG_ADDR(INT_PRINTER, i);
			
			if(*Semaforo.printer[i].s_key < 0){
								
				//Sblocco il processo sul terminale in ricezione del device richiesto
				Verhogen(Semaforo.printer[i].s_key);

				#ifdef TARGET_UMPS
					
					//Aggiorno lo status del processo svegliato
					GOODMORNING_PCB->p_s.reg_v0 = reg->status;

				#endif

				#ifdef TARGET_UARM

					//Aggiorno lo status del processo svegliato
					GOODMORNING_PCB->p_s.a1 = reg->status;


				#endif
								
			}
			
			else{

				//Invio ACK
				reg->command = CMD_ACK;
			
			}

		}
	
	}

}

//TERMINAL 7
void InterruptTerminal(){
	
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

					#ifdef TARGET_UMPS
					
					//Aggiorno lo status del processo svegliato
					GOODMORNING_PCB->p_s.reg_v0 = reg->recv_status;

				#endif

				#ifdef TARGET_UARM

					//Aggiorno lo status del processo svegliato
					GOODMORNING_PCB->p_s.a1 = reg->recv_status;


				#endif
									
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

					#ifdef TARGET_UMPS
					
					//Aggiorno lo status del processo svegliato
					GOODMORNING_PCB->p_s.reg_v0 = reg->transm_status;

				#endif

				#ifdef TARGET_UARM

					//Aggiorno lo status del processo svegliato
					GOODMORNING_PCB->p_s.a1 = reg->transm_status;


				#endif
									
				}
				
				else{

					//Invio ACK
					reg->transm_command = 1;
				
				}
				
			}	

		}
	
	}

}

