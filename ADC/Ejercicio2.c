/*
Configurar 4 canales del ADC para que funcionando en modo burst se
obtenga una frecuencia de muestreo en cada uno de 50Kmuestras/seg.
Suponer un CCLK = 100 MHz.
*/

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include <stdint.h>

void cfgGPIO()
{
	PINSEL_CFG_Type cfgCH0;
	PINSEL_CFG_Type cfgCH1;
	PINSEL_CFG_Type cfgCH2;
	PINSEL_CFG_Type cfgCH3;

	cfgCH0.Portnum = PINSEL_PORT_0;
	cfgCH1.Portnum = PINSEL_PORT_0;
	cfgCH2.Portnum = PINSEL_PORT_0;
	cfgCH3.Portnum = PINSEL_PORT_0;
	cfgCH0.Pinnum = PINSEL_PIN_23;
	cfgCH1.Pinnum = PINSEL_PIN_24;
	cfgCH2.Pinnum = PINSEL_PIN_25;
	cfgCH3.Pinnum = PINSEL_PIN_26;
	cfgCH0.Funcnum = PINSEL_FUNC_1;
	cfgCH1.Funcnum = PINSEL_FUNC_1;
	cfgCH2.Funcnum = PINSEL_FUNC_1;
	cfgCH3.Funcnum = PINSEL_FUNC_1;
	cfgCH0.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgCH1.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgCH2.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgCH3.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgCH0.OpenDrain = PINSEL_PINMODE_NORMAL;
	cfgCH1.OpenDrain = PINSEL_PINMODE_NORMAL;
	cfgCH2.OpenDrain = PINSEL_PINMODE_NORMAL;
	cfgCH3.OpenDrain = PINSEL_PINMODE_NORMAL;

	PINSEL_ConfigPin(&cfgCH0);
	PINSEL_ConfigPin(&cfgCH1);
	PINSEL_ConfigPin(&cfgCH2);
	PINSEL_ConfigPin(&cfgCH3);
}

void cfgADC(){
	ADC_Init(LPC_ADC, 200000);
	
	ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
	ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_1, ENABLE);
	ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_2, ENABLE);
	ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_3, ENABLE);
	
	ADC_BurstCmd(LPC_ADC, ENABLE);
}

int main(){
	cfgGPIO();
	cfgADC();
	while(1);
}

