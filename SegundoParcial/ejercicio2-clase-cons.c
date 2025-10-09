/*
INPUT:
- ADC canal 2
- Señal de entrada de hasta 30kHz -> cada 16 muestras se toma el promedio
- Almacenamiento por DMA

OUTPUT:
- DAC: con el promedio de las muestras
*/

/*
INPUT:
- ADC canal 2
- Señal de entrada de hasta 30kHz -> cada 16 muestras se toma el promedio
- Almacenamiento por DMA

OUTPUT:
- DAC: con el promedio de las muestras
*/

#include <LPC17xx.h>
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#include <stdint.h>

#define MAX_BUFFER 16
#define MAX_FREC 60000

static uint16_t buffer[MAX_BUFFER] = {0}; // Buffer para almacenar las muestras del ADC

void cfgPCB(){
	PINSEL_CFG_Type cfgPin;
	
	// Config pin CH2 P0.25
	cfgPin.Portnum = 0;
	cfgPin.Pinnum = 25;
	cfgPin.Funcnum = 1;
	cfgPin.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgPin.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&cfgPin);
	
	// Config pin Aout P0.26
	cfgPin.Portnum = 0;
	cfgPin.Pinnum = 26;
	cfgPin.Funcnum = 2;
	cfgPin.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgPin.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&cfgPin);
}

void cfgADC(){
    ADC_Init(LPC_ADC, MAX_FREC);
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_2, ENABLE);
    ADC_BurstCmd(LPC_ADC, ENABLE);
}

void cfgGPDMA(){
    GPDMA_Init();

    GPDMA_Channel_CFG_Type channel;
    GPDMA_LLI_Type list;

    list.SrcAddr = (uint32_t)&LPC_ADC->ADGDR;
    list.DstAddr = (uint32_t) buffer;
    list.NextLLI = (uint32_t)&list;
    list.Control = (MAX_BUFFER)
              | (GPDMA_WIDTH_HALFWORD << 18)   // Dest width = 16 bits
              | (GPDMA_WIDTH_HALFWORD << 21)   // Src width = 16 bits
              & ~(1 << 26)                      // Source increment disable
              | (1 << 27)                      // Destination increment enable
              | (1 << 31);                     // Terminal count interrupt enable


    channel.ChannelNum = 0;
    channel.SrcMemAddr = 0;
    channel.DstMemAddr = list.DstAddr;
    channel.TransferSize = MAX_BUFFER;
    channel.TransferType = GPDMA_TRANSFERTYPE_P2M;
    channel.SrcConn = GPDMA_CONN_ADC;
    channel.DstConn = 0;
    channel.DMALLI = (uint32_t)&list;

    GPDMA_Setup(&channel);
    GPDMA_ChannelCmd(0, ENABLE);
    NVIC_EnableIRQ(DMA_IRQn);
}

void cfgDAC(){
    DAC_Init(LPC_DAC);
    DAC_CONVERTER_CFG_Type cfgDAC;
	cfgDAC.CNT_ENA = DISABLE;
	cfgDAC.DBLBUF_ENA = DISABLE;
	cfgDAC.DMA_ENA = ENABLE;
	DAC_ConfigDAConverterControl(LPC_DAC, &cfgDAC);
}

int main(){
    cfgPCB();
    cfgADC();
    cfgDAC();
    cfgGPDMA();
    while(1);
}

void DMA_IRQHandler(){
    if(GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 0)){
        // Calcular el promedio
        uint32_t sum = 0;
        for(int i = 0; i < MAX_BUFFER; i++){
            // Desplazo 4 bits a la derecha para descartar los bits de estado
            // y aplico una mascara para quedarme con los 12 bits de datos
            sum += (buffer[i] >> 4) & 0x0FFF; // Extraer los 12 bits de datos
        }
        uint32_t average = sum / MAX_BUFFER;

        // Convertir el promedio a un valor adecuado para el DAC (10 bits)
        // Descarto los 2 bits menos significativos
        uint16_t dac_result = ((average >> 2) & 0x3FF);

        // Enviar al DAC
        DAC_UpdateValue(LPC_DAC, dac_result);
    }

    GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 0);

}