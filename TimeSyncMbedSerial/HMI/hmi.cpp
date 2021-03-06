/*
 * uartHMI.c
 *
 * Created: 12/05/2016 22:11:48
 *  Author: MORAND Baptiste
 */ 
#include "hmi.h"
/************************************************************************/
/* GLOBAL                                                               */
/************************************************************************/
xQueueHandle uartQueue;
Serial pc(USBTX,USBRX);
/************************************************************************/
/* FUNCTION                                                             */
/************************************************************************/
void configureHMI(void){
 
}
void HMITask(){
	
	//Can receive up to 10 char[50]
	uartQueue = xQueueCreate( LENGTH_QUEUE, sizeof( char* ) );

	while(1){
		//pc.printf("test");
			//puts("HMI task : \r\n");
		 if( uartQueue != 0 )
		 {
			 char* receive;
			 // Receive a message on the created queue.
			 while( xQueueReceive( uartQueue, &( receive ), (  portTickType ) 0) )
			 {			
				 pc.printf(receive);
				 pc.printf("\r\n");
			 }
			 

			//Clock timeCopy;
		//	timeCopy=readClock(timeCopy);
			//Clock timeCopy=timeProt.correction.sumOffset;
			//unsigned int ti=timeSoftCor;
			//unsigned int to=timeCorrecionhms;

			//printf("						t: %lus,%u\r\n",timeCopy.second,(unsigned int)timeCopy.halfmillis/2);
			//printf("timecor : %d\r\n",ti);
			//printf("corhms : %d\r\n",to);
		 }
        vTaskDelay(500/portTICK_RATE_MS);
	}
}
bool sendHMI(const char send[]){
			if( uartQueue != NULL )
			{
				if( xQueueSendToBack( uartQueue,( void * ) &send,(  portTickType ) 0 ) == pdPASS )
				{
					return true;
				}
				/* Failed to post the message. */
			}
			
				return false;
			
}