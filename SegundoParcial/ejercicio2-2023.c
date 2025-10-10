/*
Programar el microcontrolador LPC1769 en un código de lenguaje C para que mediante
su ADC digitalice una señal analógica cuyo ancho de banda es de 16 Khz. 
La señal analógica tiene una amplitud de pico máxima positiva de 3.3 voltios.

Los datos deben ser guardados utilizando el Hardware GDMA en la primera mitad de la
memoria SRAM ubicada en el bloque AHB SRAM - bank 0, de manera tal que permita almacenar
todos los datos posibles que esta memoria nos permita. Los datos deben ser almacenados
como un buffer circular conservando siempre las últimas muestras.

Por otro lado se tiene una forma de onda como se muestra en la imagen a continuación.
Esta señal debe ser generada por una función y debe ser reproducida por el DAC desde
la segunda mitad de AHB SRAM - bank 0 memoria utilizando DMA de tal forma que se logre
un periodo de 614us logrando la máxima resolución y máximo rango de tensión.

Durante operación normal se debe generar por el DAC la forma de onda mencionada como wave_form.
Se debe indicar cuál es el mínimo incremento de tensión de salida de esa forma de onda.

Cuando interrumpe una extint conectada a un pin, el ADC configurado debe completar el ciclo de
conversión que estaba realizando, y ser detenido, a continuación se comienzan a sacar las muestras
del ADC por el DAC utilizando DMA y desde las posiciones de memoria originales.

Cuando interrumpe nuevamente en el mismo pin, se vuelve a repetir la señal del DAC generada por
la forma de onda de wave_form previamente almacenada y se arranca de nuevo la conversión de datos del ADC.
Se alterna así entre los dos estados del sistema con cada interrupción externa.

Suponer una frecuencia de core clk de 80 Mhz. El código debe estar debidamente comentado.
*/

#include <LPC17xx.h>
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"

#define PRESCALER_VALUE 1000
#define BUFFER_SIZE 0x2000 / sizeof(uint32_t)
#define SRAM0 0x2007C000
#define SRAM0_HALF SRAM0 + 0x2000

uint16_t adc_samples[BUFFER_SIZE];
uint32_t dac_wave[BUFFER_SIZE];
uint8_t mode = 1; // true: wave, false: adc

void waveform(){
    for(uint16_t i=0; i<BUFFER_SIZE; i++){
        dac_wave[i] = (i*1023)/BUFFER_SIZE;
    }
}

void cfgPCB(){
    PINSEL_CFG_Type cfgPin = {0};
    
    // cfg ADC - CH0
    cfgPin.Portnum = 0;
    cfgPin.Pinnum = 23;
    cfgPin.Funcnum = 1;
    cfgPin.Pinmode = PINSEL_PINMODE_TRISTATE;
    cfgPin.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&cfgPin);

    // cfg DAC - AOUT
    cfgPin.Portnum = 0;
    cfgPin.Pinnum = 26;
    cfgPin.Funcnum = 2;
    cfgPin.Pinmode = PINSEL_PINMODE_TRISTATE;
    cfgPin.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&cfgPin);

    // cfg EINT0
    cfgPin.Portnum = 2;
    cfgPin.Pinnum = 10;
    cfgPin.Funcnum = 1;
    cfgPin.Pinmode = PINSEL_PINMODE_PULLDOWN;
    cfgPin.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&cfgPin);
}

void cfgADC(){
    ADC_Init(LPC_ADC, 200000);
    ADC_ChannelCmd(LPC_ADC, 0, ENABLE);
    ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
    NVIC_DisableIRQ(ADC_IRQn);
    ADC_BurstCmd(LPC_ADC, ENABLE);
    NVIC_SetPriority(ADC_IRQn, 1);
}

void cfgDAC(){
    DAC_Init(LPC_DAC);
    DAC_CONVERTER_CFG_Type cfgDAC = {0};
    cfgDAC.DBLBUF_ENA = DISABLE;
    cfgDAC.CNT_ENA = ENABLE;
    cfgDAC.DMA_ENA = ENABLE;
    DAC_ConfigDAConverterControl(LPC_DAC, &cfgDAC);
    DAC_SetDMATimeOut(LPC_DAC, 5);
}

void cfgEINT(){
    EXTI_SetMode(EXTI_EINT0, EXTI_MODE_EDGE_SENSITIVE);
    EXTI_SetPolarity(EXTI_EINT0, EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE);
    EXTI_ClearEXTIFlag(EXTI_EINT0);
    NVIC_SetPriority(EINT0_IRQn, 0);
    NVIC_EnableIRQ(EINT0_IRQn);
}

void cfgDMA(){
    GPDMA_Init();
    GPDMA_Channel_CFG_Type cfgADC = {0};
    GPDMA_LLI_Type cfgADC_LLI = {0};

    // Configuracion canal 0 - ADC a SRAM

    cfgADC_LLI.SrcAddr = (uint32_t)&LPC_ADC->ADGDR; 
    cfgADC_LLI.DstAddr = (uint32_t)adc_samples;
    cfgADC_LLI.NextLLI = (uint32_t)&cfgADC_LLI;
    cfgADC_LLI.Control = (BUFFER_SIZE<<0)|(2<<18)|(2<<21)|(1<<27)&~(1<<26); //revisar

    cfgADC.ChannelNum = 0;
    cfgADC.SrcMemAddr = 0;
    cfgADC.DstMemAddr = (uint32_t)adc_samples;
    cfgADC.TransferSize = BUFFER_SIZE;
    cfgAdc.TransferType = GPDMA_TRANSFERTYPE_P2M;
    cfgADC.SrcConn = GPDMA_CONN_ADC;
    cfgADC.DstConn = 0;
    cfgADC.DMALLI = (uint32_t)&cfgADC_LLI;
    GPDMA_Setup(&cfgADC);
    GPDMA_ChannelCmd(0, ENABLE);

    // Configuracion canal 1 - SRAM a DAC
    GPDMA_Channel_CFG_Type cfgDAC = {0};
    GPDMA_LLI_Type cfgDAC_LLI = {0};

    cfgDAC_LLI.SrcAddr = (uint32_t)dac_wave;
    cfgDAC_LLI.DstAddr = (uint32_t)&LPC_DAC->DACR;
    cfgDAC_LLI.NextLLI = (uint32_t)&cfgDAC_LLI;
    cfgDAC_LLI.Control = (BUFFER_SIZE<<0)|(2<<18)|(2<<21)|(1<<26)&~(1<<27); //revisar

    cfgDAC.ChannelNum = 1;
    cfgDAC.SrcMemAddr = (uint32_t)dac_wave;
    cfgDAC.DstMemAddr = 0;
    cfgDAC.TransferSize = BUFFER_SIZE;
    cfgDAC.TransferType = GPDMA_TRANSFERTYPE_M2P;
    cfgDAC.SrcConn = 0;
    cfgDAC.DstConn = GPDMA_CONN_DAC;
    cfgDAC.DMALLI = (uint32_t)&cfgDAC_LLI;
    GPDMA_Setup(&cfgDAC);
}

int main(){
    cfgPCB();
    cfgEINT();
    cfgADC();
    cfgDAC();
    cfgDMA();
    waveform();
    while(1);
}

void EINT0_IRQHandler(){
    mode = !mode;
    if(mode){
        NVIC_EnableIRQ(ADC_IRQn);
    }else{
        NVIC_DisableIRQ(ADC_IRQn);
        GPDMA_ChannelCmd(1, DISABLE);
        GPDMA_ChannelCmd(0, ENABLE);
    }
    EXTI_ClearEXTIFlag(EXTI_EINT0);
}

void ADC_IRQHandler(){
    ADC_ChannelCmd(LPC_ADC, 0, DISABLE);
    GPDMA_ChannelCmd(0, DISABLE);
    GPDMA_ChannelCmd(1, ENABLE);
}