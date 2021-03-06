/*
 * internalClock.c
 *
 * Created: 13/05/2016 13:59:02
 *  Author: MORAND Baptiste
 */ 


#include "internalClock.h"
volatile unsigned int count_set_period=VALUE_TIMER;
volatile int valueCor;
volatile int timeCorrection;
volatile int timeCorr;
volatile bool stateLed=false;
DigitalOut led(LED1);
struct rtc_module rtc_instance;

uint32_t  readClock(Clock* timeClock){
	//rtc_count_disable(&rtc_instance);
	//Disable_global_interrupt();
	uint32_t timeCounter=0;
	RTC->MODE0.CTRL.reg &=  0b1111111111111101;
	while(RTC->MODE0.STATUS.bit.SYNCBUSY==1);
	
	timeCounter=RTC->MODE0.COUNT.reg; 
	
	//timeCounter=0;
	
	RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_ENABLE;
	
	timeClock->second=timeManage.second;	
	timeClock->halfmillis=timeManage.halfmillis;	
	Enable_global_interrupt();
	timeClock->sign=true;	
	
	//	rtc_count_enable(&rtc_instance); 
	timeClock->halfmillis+=timeCounter;
	timeClock->second+=timeClock->halfmillis/RTC_FREQ;
	timeClock->halfmillis=timeClock->halfmillis%RTC_FREQ;
	
	return timeCounter;
}
void writeClock(Clock clk){
	//rtc_count_disable(&rtc_instance);
	Disable_global_interrupt();
	timeManage.second=clk.second;
	timeManage.halfmillis=(int)(clk.halfmillis/VALUE_TIMER)*VALUE_TIMER;
	RTC->MODE0.CTRL.reg &=  0b1111111111111101;
	while(RTC->MODE0.STATUS.bit.SYNCBUSY==1);
	RTC->MODE0.COUNT.reg=clk.halfmillis%VALUE_TIMER;
	RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_ENABLE;
	Enable_global_interrupt();
	timeManage.second+=timeManage.halfmillis/RTC_FREQ;
	timeManage.halfmillis=timeManage.halfmillis%RTC_FREQ;
	
	
}
void isrInternalClok(void){
			Disable_global_interrupt();
			//printf("+");
			static bool state;
			state=stateLed;
			

			timeManage.halfmillis+=VALUE_TIMER;

		
			if(timeManage.halfmillis<(RTC_FREQ/2)){//all 500ms
				stateLed=LOW;
			}
			else{
				stateLed=HIGH;
			}
			if(state!=stateLed){
				led = stateLed;
				//	if(synchroLed!=NULL){
				//		xSemaphoreGiveFromISR( synchroLed,NULL);
				//	}
			}

			
		timeManage.second+=timeManage.halfmillis/RTC_FREQ;
		timeManage.halfmillis=timeManage.halfmillis%RTC_FREQ;
			if((valueCor!=0)){
				if (timeCorr==0){
					timeCorr=timeCorrection;
					Clock off;
					off.second=(unsigned int )abs(valueCor)/RTC_FREQ;
					off.halfmillis=(unsigned int)abs(valueCor)%RTC_FREQ;
					if(valueCor<0){
						off.sign=false;
					}
					else{
						off.sign=true;
					}
					//change the time
					timeManage=sumClock(timeManage,off);
					//change the correction offset
					sumOffset=sumClock(sumOffset,off);//ok
				}
				else{
					timeCorr--;
				}	
				sumOffset.second+=sumOffset.halfmillis/RTC_FREQ;
				sumOffset.halfmillis=sumOffset.halfmillis%RTC_FREQ;
				timeManage.second+=timeManage.halfmillis/RTC_FREQ;
				timeManage.halfmillis=timeManage.halfmillis%RTC_FREQ;
			
			}
			

			
	Enable_global_interrupt();
}
void RTC_Handler( void ){
	RTC->MODE0.INTFLAG.bit.CMP0=1;		//disable the flag COMPARE
	RTC->MODE0.INTFLAG.bit.OVF=1;		//disable the flag COMPARE
	isrInternalClok();
}
void configureInternalClock(void){
	//gENERIC CLOCK GENRERATOR
	GCLK->GENCTRL.reg=0b00000000000000010000010000000010;
	//Generic Clock Generator Division
	GCLK->GENDIV.reg= 0b00000000000000000000000100000010;
	GCLK->CLKCTRL.reg=0x4204;
	//peripheral configuration
	
	RTC->MODE0.CTRL.bit.PRESCALER=0x0;	//prescaller at 0
	RTC->MODE0.CTRL.bit.MODE=0x0;			//mode 0 : 32 bit counter
	RTC->MODE0.CTRL.bit.SWRST=0;			//no software reset
	RTC->MODE0.CTRL.bit.MATCHCLR=1;			//automatic reset counter
	
	//event configuration
	RTC->MODE0.INTENSET.bit.OVF=1;		//enable the event overflow
	RTC->MODE0.INTENSET.bit.CMP0=1;		//enable the event compare
	RTC->MODE0.INTFLAG.bit.CMP0=1;		//disable the flag COMPARE
	RTC->MODE0.INTFLAG.bit.OVF=1;		//disable the flag COMPARE
	RTC->MODE0.COUNT.bit.COUNT=0;			//set the RTC value
	RTC->MODE0.COMP[0].reg = count_set_period;

	RTC->MODE0.CTRL.reg |= RTC_MODE0_CTRL_ENABLE;

	NVIC_SetPriority(RTC_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
	NVIC_EnableIRQ(RTC_IRQn);
}