#include "mbed.h"


 




#include <utask.h>
#include <hmi.h>
#include <ktrap.h>
#include <internalClock.h>
#include <ppsGPS.h>
#include <network.h>

int main (void ){
	 system_interrupt_disable_global();
    //configure HMI
    configureHMI();
    //RTC configure
	printf("internal clock config\r\n");
	configureInternalClock();
    //configure external intrerrupt for pps
	printf("pps config\r\n");
   configurationPPS();
    //configure the network of the time SYNCHRONISATION
	printf("network config\r\n");
   configurationNetwork();
	printf("kernel config\r\n");
    // Initialize the demo..
    kernelConfig();
    //demask global interrupt
    system_interrupt_enable_global();
    printf("\r\n\r\n**********************************************************************\r\n");
    printf("tick time: %lu ms\r\n",portTICK_RATE_MS);   
    #ifdef MASTERMODE
        pc.printf("Master clock\r\n");
    #else
        pc.printf("Slave Clock \r\n");
    #endif
    pc.printf("end config application begin\r\n");
    pc.printf("\r\n**********************************************************************\r\n\r\n");
    //start the scheduler
    vTaskStartScheduler();
    while(1){//trap
    }

}
