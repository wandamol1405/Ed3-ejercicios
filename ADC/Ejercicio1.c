/*
Adquirir dos señales analogica de 16kHz por el pin P0.23 y P0.24 cada 1s.
Utilizar la funcionalidad de MATCH del pin P1.29 como base de
tiempo para la adquisición.
*/

#include "LPC17xx.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include <stdint.h>


void cfgTimer(){
	TIM_TIMERCFG_Type cfgTimerMode;
	TIM_MATCHCFG_Type cfgTimerMatch;

	cfgTimerMode.PrescaleOption = TIM_PRESCALE_USVAL;
	cfgTimerMode.PrescaleValue = 1000;

	cfgTimerMatch.MatchChannel = 1;
	cfgTimerMatch.MatchValue = 499; 
	cfgTimerMatch.IntOnMatch = DISABLE;
	cfgTimerMatch.ResetOnMatch = ENABLE;
	cfgTimerMatch.StopOnMatch = DISABLE;
	cfgTimerMatch.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &cfgTimerMode);
	TIM_ConfigMatch(LPC_TIM0, &cfgTimerMatch);
	TIM_Cmd(LPC_TIM0, ENABLE);
}

void cfgGPIO()
{
	PINSEL_CFG_Type cfgPinADC0_CH0;
	PINSEL_CFG_Type cfgPinADC0_CH1;
	PINSEL_CFG_Type cfgPinMAT0_CH1;

	cfgPinADC0_CH0.Portnum = PINSEL_PORT_0;
	cfgPinADC0_CH0.Pinnum = PINSEL_PIN_23;
	cfgPinADC0_CH0.Funcnum = PINSEL_FUNC_1;
	cfgPinADC0_CH0.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgPinADC0_CH0.OpenDrain = PINSEL_PINMODE_NORMAL;

	cfgPinADC0_CH1.Portnum = PINSEL_PORT_0;
	cfgPinADC0_CH1.Pinnum = PINSEL_PIN_24;
	cfgPinADC0_CH1.Funcnum = PINSEL_FUNC_1;
	cfgPinADC0_CH1.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgPinADC0_CH1.OpenDrain = PINSEL_PINMODE_NORMAL;

	cfgPinMAT0_CH1.Portnum = PINSEL_PORT_1;
	cfgPinMAT0_CH1.Pinnum = PINSEL_PIN_29;
	cfgPinMAT0_CH1.Funcnum = PINSEL_FUNC_3;
	cfgPinMAT0_CH1.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgPinMAT0_CH1.OpenDrain = PINSEL_PINMODE_NORMAL;

	PINSEL_ConfigPin(&cfgPinADC0_CH0);
	PINSEL_ConfigPin(&cfgPinADC0_CH1);
	PINSEL_ConfigPin(&cfgPinMAT0_CH1);

}

void cfgADC(){
	ADC_Init(LPC_ADC, 64000);
	ADC_BurstCmd(LPC_ADC, DISABLE);
	ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_FALLING);
	ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
	ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_1, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
	NVIC_EnableIRQ(ADC_IRQn);
	ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT01);
}

int main(){
	cfgGPIO();
	cfgTimer();
	cfgADC();
	while(1);
}


void ADC_IRQHandler(void){
	if(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_0, ADC_DATA_DONE)){
		uint16_t ADC0Value_CH0 = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);
	}
	if(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_1, ADC_DATA_DONE)){
		uint16_t ADC0Value_CH1 = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_1);
	}
}
