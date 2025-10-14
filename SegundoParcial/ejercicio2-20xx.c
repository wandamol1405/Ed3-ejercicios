/*
Se tienen tres bloques de datos de 4KBytes de longitud cada uno en el cual se han guardado tres formas de onda.
Cada muestra de la onda es un valor de 32 bits que se ha capturado desde el ADC. Las direcciones de inicio de cada
bloque son representadas por macros del estilo DIRECCION_BLOQUE_N, con N=0,1,2.

Se pide que, usando DMA y DAC se genere una forma de onda por la salida analógica de la LPC1769.
La forma de onda cambiará en función de una interrupción externa conectada a la placa de la siguiente manera:

- 1er interrupción: Forma de onda almacenada en bloque 0, con frecuencia de señal de 60[KHz].
- 2da interrupción: Forma de onda almacenada en bloque 1 con frecuencia de señal de 120[KHz].
- 3ra interrupción: Forma de onda almacenada en bloque 0 y bloque 2 (una a continuación de la otra) con frecuencia
 	 	 	 	 	 de señal de 450[KHz].
- 4ta interrupción: Vuelve a comenzar con la forma de onda del bloque 0.

En cada caso se debe utilizar el menor consumo de energía posible del DAC.
*/

#include <LPC17xx.h>
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_gpdma.h"

#define DIRECCION_BLOQUE_0 0x2007C000
#define DIRECCION_BLOQUE_1 0x2007E000
#define DIRECCION_BLOQUE_2 0x20080000

#define BLOQUE_SIZE 1024

void cfgPCB(){
	PINSEL_CFG_Type cfgPin;

	// Configuro el canal 0 del ADC
	cfgPin.Portnum = 0;
	cfgPin.Pinnum = 23;
	cfgPin.Funcnum = 1;
	cfgPin.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgPin.OpenDrain = PINSEL_PINMODE_NORMAL;

	PINSEL_ConfigPin(&cfgPin);

	// Configuro la salida analogica del DAC
	cfgPin.Portnum = 0;
	cfgPin.Pinnum = 26;
	cfgPin.Funcnum = 2;
	cfgPin.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgPin.OpenDrain = PINSEL_PINMODE_NORMAL;

	PINSEL_ConfigPin(&cfgPin);

	// Configuro la interrupcion externa EINT0
	cfgPin.Portnum = 2;
	cfgPin.Pinnum = 10;
	cfgPin.Funcnum = 1;
	cfgPin.Pinmode = PINSEL_PINMODE_PULLDOWN;
	cfgPin.OpenDrain = PINSEL_PINMODE_NORMAL;

	PINSEL_ConfigPin(&cfgPin);
}

void cfgEINT(){
	EXTI_SetMode(EXTI_EINT0, EXTI_MODE_EDGE_SENSITIVE);
	EXTI_SetPolarity(EXTI_EINT0, EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE);
	EXTI_ClearEXTIFlag(EXTI_EINT0);
	NVIC_EnableIRQ(EINT0_IRQn);
}

void cfgDAC(){
	DAC_Init(LPC_DAC);

	DAC_CONVERTER_CFG_Type cfgDAC = {0};

	cfgDAC.DBLBUF_ENA = DISABLE;
	cfgDAC.CNT_ENA = ENABLE;
	cfgDAC.DMA_ENA = ENABLE;

	DAC_SetBias(LPC_DAC, 1);
	DAC_ConfigDAConverterControl(LPC_DAC, &cfgDAC);
}

void cfgDMA(){
	GPDMA_Init();

	GPDMA_Channel_CFG_Type cfgChannel = {0};
	GPDMA_LLI_Type cfgLLI = {0};
	GPDMA_LLI_Type cfgLLI2 = {0};

	// Canal 0: 1ra interrupcion
	cfgLLI.SrcAddr = (uint32_t)DIRECCION_BLOQUE_0;
	cfgLLI.DstAddr = (uint32_t)&LPC_DAC->DACR;
	cfgLLI.NextLLI = (uint32_t)&cfgLLI;
	cfgLLI.Control = (BLOQUE_SIZE>>0)|(1<<12)|(1<<15)|(2<<18)|(2<<21)|(1<<26);

	cfgChannel.ChannelNum = 0;
	cfgChannel.SrcMemAddr = DIRECCION_BLOQUE_0;
	cfgChannel.DstMemAddr = 0;
	cfgChannel.TransferSize = BLOQUE_SIZE;
	cfgChannel.TransferType = GPDMA_TRANSFERTYPE_M2P;
	cfgChannel.SrcConn = 0;
	cfgChannel.DstConn = GPDMA_CONN_DAC;
	cfgChannel.DMALLI = (uint32_t)&cfgLLI;

	GPDMA_Setup(&cfgChannel);

	// Canal 1: 2da interrupcion
	cfgLLI2.SrcAddr = (uint32_t)DIRECCION_BLOQUE_1;
	cfgLLI2.DstAddr = (uint32_t)&LPC_DAC->DACR;
	cfgLLI2.NextLLI = (uint32_t)&cfgLLI2;
	cfgLLI2.Control = (BLOQUE_SIZE>>0)|(1<<12)|(1<<15)|(2<<18)|(2<<21)|(1<<26);

	cfgChannel.ChannelNum = 1;
	cfgChannel.SrcMemAddr = DIRECCION_BLOQUE_1;
	cfgChannel.DMALLI = (uint32_t)&cfgLLI2;

	GPDMA_Setup(&cfgChannel);

	// Canal 2: 3ra interrupcion
	cfgLLI.NextLLI = (uint32_t)&cfgLLI2;
	cfgLLI2.NextLLI = (uint32_t)&cfgLLI;

	cfgChannel.ChannelNum = 2;
	cfgChannel.SrcMemAddr = DIRECCION_BLOQUE_0;
	cfgChannel.DMALLI = (uint32_t)&cfgLLI;

	GPDMA_Setup(&cfgChannel);

}

int main(){
	cfgPCB();
	cfgEINT();
	cfgDAC();
	cfgDMA();
	while(1);
}

void EINT0_IRQHandler(){
	static uint8_t count = 0;
	count = (count+1)%4;

	switch(count){
	case 1:
		GPDMA_ChannelCmd(2, DISABLE);
		GPDMA_ChannelCmd(0, ENABLE);
		DAC_UpdateValue(LPC_DAC, 416);
		break;
	case 2:
		GPDMA_ChannelCmd(0, DISABLE);
		GPDMA_ChannelCmd(1, ENABLE);
		DAC_UpdateValue(LPC_DAC, 207);
		break;
	case 3:
		GPDMA_ChannelCmd(1, DISABLE);
		GPDMA_ChannelCmd(2, ENABLE);
		DAC_UpdateValue(LPC_DAC, 54);
		break;
	}
	EXTI_ClearEXTIFlag(EXTI_EINT0);
}
