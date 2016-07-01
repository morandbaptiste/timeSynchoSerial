/*
 * uart.c
 *
 * Created: 18/05/2016 18:24:38
 *  Author: MORAND Baptiste
 */
#include "network.h"
/************************************************************************/
/* GLOBAL                                                               */
/************************************************************************/
struct usart_module usart_instance;
 uint16_t rx;
volatile circularBuffer buffer;
/************************************************************************/
/* GLOBAL FUNCTION                                                      */
/************************************************************************/
void ISRNetworkReception(void){
	
	//Disable_global_interrupt();
	usart_read_job(&usart_instance,&rx);
	//synchro semaphore with a task
	if(timeProt.synchroTime!=NULL){
		xSemaphoreGiveFromISR(timeProt.synchroTime,NULL);
	}
	
	buffer.buffer[buffer.indice]=rx;
	
	buffer.indice++;
	if(buffer.indice>=CIRCULAR_BUFFER_LENGTH){
		buffer.indice=0;
	}

	if(rx==HEADER){
		timeProt.waitIdentifier=true;

		#ifdef MASTERMODE
			Clock save;
			save.second=timeProt.saveTime[0].second;
			save.halfmillis=timeProt.saveTime[0].halfmillis;
			save.sign=timeProt.saveTime[0].sign;
			readClock(&save);
			timeProt.saveTime[0].second=save.second;
			timeProt.saveTime[0].halfmillis=save.halfmillis;
			timeProt.saveTime[0].sign=save.sign;
		
		#else
			readClock(&timeProt.rx);
		#endif
		return;
	}
	#ifdef MASTERMODE
	if(timeProt.waitIdentifier==true){
		timeProt.waitIdentifier=false;
		
		if(rx<MAX_SLAVE_CLOCK){
			timeProt.saveTime[rx].second=timeProt.saveTime[0].second;
			timeProt.saveTime[rx].halfmillis=timeProt.saveTime[0].halfmillis;
		}
		return;
	}
	
	#else
	if(timeProt.waitIdentifier==true){
		timeProt.waitIdentifier=false;
		if(rx==0){
			timeProt.waitType=true;
		}
	}
	if(timeProt.waitType==true){
		if(rx==DELAYRESPONSE){
			timeProt.rxDelay.second=timeProt.rx.second;
			timeProt.rxDelay.halfmillis=timeProt.rx.halfmillis;
		}
		else{
			if(rx==SYNC){
				timeProt.rxSync.second=timeProt.rx.second;
				timeProt.rxSync.halfmillis=timeProt.rx.halfmillis;
			}
		}
		
	}

	#endif
	
	//Enable_global_interrupt();

}

void ISRNetworkTransmission(void){
	//puts("t\r\n");
	
}

bool networkAvailable(void){


	if(buffer.currentIndice!=buffer.indice){
		//puts("network available");
		return true;	
	}
	else{
			
		return false;
	}

}
void networkFlush(void){
	//usart_disable_callback(&usart_instance, USART_CALLBACK_BUFFER_RECEIVED);//momo
	buffer.currentIndice=buffer.indice;
	//usart_enable_callback(&usart_instance, USART_CALLBACK_BUFFER_RECEIVED); //momo
}
void networkTx(const uint8_t send[],const uint8_t length){
	//usart_unlock(&usart_instance);
	
		usart_write_buffer_wait(&usart_instance, send,length);
		//Enable_global_interrupt();
		//usart_write_buffer_job(&usart_instance, send, length);
}
uint8_t networkRead(void){
	if(networkAvailable()){
		uint8_t recu;
		recu=buffer.buffer[buffer.currentIndice];
		buffer.currentIndice++;
		if(buffer.currentIndice>=CIRCULAR_BUFFER_LENGTH){
			buffer.currentIndice=0;
		}
		return recu; 
	}
	else{
		return 0;
	}

	
}

void configurationNetwork(){
	buffer.currentIndice=0;
	buffer.indice=0;
	struct usart_config config_usart;
	//8-bit asynchronous USART
	//No parity
	//One stop bit
	//9600 baud
	//Transmitter enabled
	//Receiver enabled
	//GCLK generator 0 as clock source
	//Default pin configurationc
	usart_get_config_defaults(&config_usart);
	//baudrate 9600
	
	config_usart.baudrate    = 19200;
	config_usart.mux_setting = USART_RX_1_TX_0_XCK_1 ;//Rx on PAD1 Tx on PAD0
	config_usart.pinmux_pad0 = PINMUX_PA16D_SERCOM3_PAD0;//MUX D
	config_usart.pinmux_pad1 = PINMUX_PA17D_SERCOM3_PAD1;// MUX D
	config_usart.pinmux_pad2 = PINMUX_UNUSED;
	config_usart.pinmux_pad3 = PINMUX_UNUSED;
	
	while (usart_init(&usart_instance,SERCOM3, &config_usart) != STATUS_OK) ;

	usart_enable(&usart_instance);
	//config callback
		//config register
	usart_register_callback(&usart_instance,(usart_callback_t)ISRNetworkTransmission, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_register_callback(&usart_instance,(usart_callback_t)ISRNetworkReception, USART_CALLBACK_BUFFER_RECEIVED);

		//enable callback
	usart_enable_callback(&usart_instance, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_enable_callback(&usart_instance, USART_CALLBACK_BUFFER_RECEIVED);
	usart_read_job(&usart_instance,&rx);

}
