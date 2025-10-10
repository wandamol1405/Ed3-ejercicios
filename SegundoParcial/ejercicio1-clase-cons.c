/*
INPUT:
- Sensor lineal: 0-100 grados
- Trigger: MAT0.1 (cada 100ms)

OUTPUT:
- LED:
	- VERDE: 0-40 grados
	- AMARILLO: 41-60 grados
	- ROJO: 61-100 grados && 10 muestras consecutivas en el rango
*/

#include <LPC17xx.h>
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_adc.h"
#include <stdint.h>

#define PRESCALER_VALUE 1000
#define MATCH_VALUE 50 - 1

#define RED 22  //p0.22
#define GREEN 25 //p3.25

#define UMBRAL_1 40*(4065/100) // 40 grados 
#define UMBRAL_2 60*(4065/100) // 60 grados 

#define MAX_SAMPLES 10

uint16_t count_samples;

void cfgPCB(){
	PINSEL_CFG_Type cfgPin;
	// cfg MAT1.1
	cfgPin.Portnum = 1;
	cfgPin.Pinnum = 29;
	cfgPin.Funcnum = 3;
	cfgPin.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgPin.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&cfgPin);

	// cfg ADC - CH0
	cfgPin.Portnum = 0;
	cfgPin.Pinnum = 23;
	cfgPin.Funcnum = 1;
	cfgPin.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgPin.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&cfgPin);

	// cfg LED RED
	cfgPin.Portnum = 0;
	cfgPin.Pinnum = 22;
	cfgPin.Funcnum = 0;
	cfgPin.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgPin.OpenDrain = PINSEL_PINMODE_NORMAL;
	GPIO_SetDir(cfgPin.Portnum, cfgPin.Pinnum, 1);
	PINSEL_ConfigPin(&cfgPin);

	// cfg LED GREEN
	cfgPin.Portnum = 3;
	cfgPin.Pinnum = 25;
	cfgPin.Funcnum = 0;
	cfgPin.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgPin.OpenDrain = PINSEL_PINMODE_NORMAL;
	GPIO_SetDir(cfgPin.Portnum, cfgPin.Pinnum, 1);
	PINSEL_ConfigPin(&cfgPin);
}

void cfgADC(){
	ADC_Init(LPC_ADC, 100000); // 100 kHz
	ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_RISING); // Comienzo en flanco de subida
	ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE); // Habilito canal 0
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE); // Habilito interrupcion por canal 0
	NVIC_EnableIRQ(ADC_IRQn); // Habilito interrupciones
    ADC_BurstCmd(LPC_ADC, DISABLE); // Deshabilito modo burst
	ADC_StartCmd(LPC_ADC, ENABLE); // Habilito inicio de conversion
}

void cfgTimer(){
	TIM_TIMERCFG_Type cfgTimer; 
	TIM_MATCHCFG_Type cfgMatch;

	cfgTimer.PrescaleOption = TIM_PRESCALE_USVAL; // Prescaler en microsegundos
	cfgTimer.PrescaleValue = PRESCALER_VALUE; // 1000 -> 1ms

	cfgMatch.MatchChannel = 0; // Canal 0
	cfgMatch.IntOnMatch = DISABLE; // No habilito interrupcion
	cfgMatch.ResetOnMatch = ENABLE; // Reseteo el timer cuando llega a match
	cfgMatch.StopOnMatch = DISABLE; // No detengo el timer cuando llega a match
	cfgMatch.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE; // Cambio el estado del pin MAT0.1 cuando llega a match
	cfgMatch.MatchValue = MATCH_VALUE; // 50 -> 50ms

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &cfgTimer);
	TIM_ConfigMatch(LPC_TIM0, &cfgMatch);
	TIM_Cmd(LPC_TIM0, ENABLE);
}

int main(){
	cfgPCB();
	cfgTimer();
	cfgADC();
	while(1);
}

void ADC_IRQHandler(){
	if(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_0, ADC_DATA_DONE)){ 
		uint16_t ADCValue = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);
		if(ADCValue <= UMBRAL_1){
			GPIO_SetValue(3, GREEN);
			GPIO_ClearValue(1, RED);
		}else if(ADCValue > UMBRAL_1 && ADCValue <= UMBRAL_2){
			GPIO_SetValue(3, GREEN);
			GPIO_SetValue(1, RED);
		}else if(ADCValue > UMBRAL_2){
			if(count_samples == MAX_SAMPLES-1){
				GPIO_ClearValue(3, GREEN);
				GPIO_SetValue(1, RED);
				count_samples = 0;
			}else{
				count_samples++;
			}
		}
	}
}
