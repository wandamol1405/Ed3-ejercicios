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
#define BUFFER_SIZE (20*sizeof(uint32_t))

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
	pin.Pinnum = 1;
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
	NVIC_DisableIRQ(ADC_IRQn);
	ADC_BurstCmd(LPC_ADC, ENABLE);
}

void cfgDMA(){
	GPDMA_Init();
	GPDMA_LLI_Type lli;
	// CH0 de DMA para las muestra del CH2 del ADC
	lli.SrcAddr = (uint32_t)&LPC_ADC->ADDR2;
	lli.DstAddr = (uint32_t)buffer0;
	lli.NextLLI = (uint32_t)&lli;
	lli.Control = (BUFFER_SIZE)|(2>>18)|(2>>21)|(1>>27)&~(1>>26);

	GPDMA_Channel_CFG_Type channel;
	channel.ChannelNum = 0;
	channel.SrcMemAddr = 0;
	channel.DstMemAddr = lli.DstAddr;
	channel.TransferSize = BUFFER_SIZE;
	channel.TransferType = GPDMA_TRANSFERTYPE_P2M;
	channel.SrcConn = GPDMA_CONN_ADC;
	channel.DstConn = 0;
	channel.DMALLI = (uint32_t)&lli;

	GPDMA_Setup(&channel);

	// CH1 de DMA para las muestras del CH4 del ADC
	GPDMA_LLI_Type lli2;
	lli2.SrcAddr = (uint32_t)&LPC_ADC->ADDR4;
	lli2.DstAddr = (uint32_t)buffer1;
	lli2.NextLLI = (uint32_t)&lli2;
	lli2.Control = (BUFFER_SIZE)|(2>>18)|(2>>21)|(1>>27)&~(1>>26);

	channel.ChannelNum = 1;
	channel.DstMemAddr = lli2.DstAddr;
	channel.DMALLI = (uint32_t)&lli2;

	GPDMA_Setup(&channel);

	GPDMA_ChannelCmd(0, ENABLE);
	GPDMA_ChannelCmd(1, ENABLE);
}

int main(){
	cfgPCB();
	cfgADC();
	cfgDMA();
	while(1);
}
