/*
Programar el microcontrolador LPC1769 para que mediante su ADC digitalice dos señales
analogicas cuyos anchos de bandas son de 10kHz cada una. Los canales utilizados deben
ser el 2 y el 4 y los datos deben ser guardados en dos regiones de memorias distintas
que permitan contar con los 20 datos de cada canal. Suponer una frecuencia de core cclk
de 100MHz. El codigo debe estar debidamente comentado.
*/

#include <LPC17xx.h>
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_gpdma.h"

#define FREQ_ADC 40000 // 40kHz
#define SRAM0 0x2007C000
#define SRAM1 0x20080000
#define BUFFER_SIZE 20

uint32_t buffer0[BUFFER_SIZE];
uint32_t buffer1[BUFFER_SIZE];

void cfgPCB(){
	PINSEL_CFG_Type pin;

	//CH2 del ADC -> P0.25 F1
	pin.Portnum = 0;
	pin.Pinnum = 25;
	pin.Funcnum = 1;
	pin.Pinmode = PINSEL_PINMODE_TRISTATE;
	pin.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&pin);

	//CH4 del ADC -> P1.30 f3
	pin.Portnum = 1;
	pin.Pinnum = 30;
	pin.Funcnum = 3;
	pin.Pinmode = PINSEL_PINMODE_TRISTATE;
	pin.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&pin);
}

void cfgADC(){
	ADC_Init(LPC_ADC, FREQ_ADC);
	ADC_ChannelCmd(LPC_ADC, 2, ENABLE);
	ADC_ChannelCmd(LPC_ADC, 4, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN2, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN4, ENABLE);
	NVIC_EnableIRQ(ADC_IRQn);
	ADC_BurstCmd(LPC_ADC, ENABLE);
}

int main(){
	cfgPCB();
	cfgADC();
	cfgDMA();
	while(1);
}

void ADC_IRQHandler(){
	static uint8_t index = 0;
	index = (index+1) % BUFFER_SIZE;
	if(ADC_ChannelGetStatus(LPC_ADC, 2, ADC_DATA_DONE)){
		buffer0[index] = ADC_ChannelGetData(LPC_ADC, 2);
	}
	if(ADC_ChannelGetStatus(LPC_ADC, 4, ADC_DATA_DONE)){
		buffer1[index] = ADC_ChannelGetData(LPC_ADC, 4);
	}
	NVIC_ClearPendingIRQ(ADC_IRQn);
}
