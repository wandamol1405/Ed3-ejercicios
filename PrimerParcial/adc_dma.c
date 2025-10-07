/*
Adquirir una señal analogica por el pin P0.23 y almacenar en un buffer de 256 muestras en la memoria AHB SRAM BANCO 1 (0x20080000 - 0x20083FFF).
*/

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpdma.h"
#include <stdint.h>

uint16_t *buffer = (uint16_t *) 0x20080000;
GPDMA_LLI_Type dmaList = {0};

void cfgPin()
{
	PINSEL_CFG_Type cfgPinADC0_CH0;

	cfgPinADC0_CH0.Portnum = PINSEL_PORT_0;
	cfgPinADC0_CH0.Pinnum = PINSEL_PIN_23;
	cfgPinADC0_CH0.Funcnum = PINSEL_FUNC_1;
	cfgPinADC0_CH0.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgPinADC0_CH0.OpenDrain = PINSEL_PINMODE_NORMAL;

	PINSEL_ConfigPin(&cfgPinADC0_CH0);

}

void cfgADC(){
	ADC_Init(LPC_ADC, 64000); // FRECUENCIA DE 64kHz
	ADC_BurstCmd(LPC_ADC, ENABLE);
	ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
}

void cfgDMA(){
	GPDMA_Channel_CFG_Type cfgDMA;
	
	cfgDMA.ChannelNum = GPDMA_CHANNEL_0;
	cfgDMA.TransferSize = 256;
	cfgDMA.SrcMemAddr = LPC_ADC->ADGDR;
	cfgDMA.DstMemAddr = (uint32_t) buffer;
	cfgDMA.TransferType = GPDMA_TRANSFERTYPE_P2M;
	cfgDMA.SrcConn = GPDMA_CONN_ADC;
	cfgDMA.LinkedListItem = (uint32_t) &dmaList;


	dmaList.SrcAddr = LPC_ADC->ADGDR;
	dmaList.DstAddr = (uint32_t) buffer;
	dmaList.NextLLI = (uint32_t) &dmaList; // Puntero a si mismo para que sea circular
	dmaList.Control = 256 | (1<<12) | (1<<15) | (1<<18) | (1<<21) | (1<<27);

	GPDMA_Init();

	GPDMA_Setup(&cfgDMA);
	GPDMA_ChannelCmd(GPDMA_CHANNEL_0, ENABLE);

}

int main(){
	cfgPin();
	cfgADC();
	cfgDMA();
	while(1);
}

