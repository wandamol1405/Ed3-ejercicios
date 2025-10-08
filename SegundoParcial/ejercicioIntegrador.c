
#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_gpdma.h"
#include <stdint.h>

#define SRAM0 0x2007C000
#define SRAM0_HALF SRAM0 + 0x2000
#define SRAM1 0x20080000
#define TRANSFER_SIZE (0x20080000 - 0x2007C000)/2
#define DAC_MAX 1023
#define DAC_MID DAC_MAX/2
#define NUM_SAMPLES 382 // Numero de muestras para formar la onda

volatile uint32_t *adc_samples = (uint32_t *) 0x2007C000;
volatile uint32_t *dac_wave = (uint32_t *) 0x2008000;
volatile uint32_t sample_index = 0;
volatile uint8_t adc_pointer_mode = 0;

void cfgPBC(void);
void cfgADC(void);
void cfgDAC(void);
void cfgEINT(void);
void cfgTimer0(void);
void cfgDMA(void);
void generate_wave(void);

void cfgPBC(){
	PINSEL_CFG_Type cfgADC_CH0;
	PINSEL_CFG_Type cfgTMR0_MR1;
	PINSEL_CFG_Type cfgDAC_Aout;
	PINSEL_CFG_Type cfgEINT0;

	cfgADC_CH0.Portnum = 0;
	cfgADC_CH0.Pinnum = 23;
	cfgADC_CH0.Funcnum = PINSEL_FUNC_1;
	cfgADC_CH0.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgADC_CH0.OpenDrain = PINSEL_PINMODE_NORMAL;

	cfgTMR0_MR1.Portnum = 1;
	cfgTMR0_MR1.Pinnum = 29;
	cfgTMR0_MR1.Funcnum = PINSEL_FUNC_3;
	cfgTMR0_MR1.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgTMR0_MR1.OpenDrain = PINSEL_PINMODE_NORMAL;

	cfgDAC_Aout.Portnum = 0;
	cfgDAC_Aout.Pinnum = 26;
	cfgDAC_Aout.Funcnum = PINSEL_FUNC_2;
	cfgDAC_Aout.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgDAC_Aout.OpenDrain = PINSEL_PINMODE_NORMAL;

	cfgEINT0.Portnum = 2;
	cfgEINT0.Pinnum = 10;
	cfgEINT0.Funcnum = PINSEL_FUNC_1;
	cfgEINT0.Pinmode = PINSEL_PINMODE_PULLDOWN;
	cfgEINT0.OpenDrain = PINSEL_PINMODE_NORMAL;

	PINSEL_ConfigPin(&cfgADC_CH0);
	PINSEL_ConfigPin(&cfgTMR0_MR1);
	PINSEL_ConfigPin(&cfgDAC_Aout);
	PINSEL_ConfigPin(&cfgEINT0);
}

void cfgADC(){
	ADC_Init(LPC_ADC, 160000);
	ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_FALLING);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
	NVIC_EnableIRQ(ADC_IRQn);
	ADC_BurstCmd(LPC_ADC, DISABLE);
	ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT01);
}

void cfgDAC(){
	DAC_Init(LPC_DAC);
}

void cfgEINT(){
	EXTI_SetMode(EXTI_EINT0, EXTI_MODE_EDGE_SENSITIVE);
	EXTI_SetPolarity(EXTI_EINT0, EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE);
	EXTI_ClearEXTIFlag(EXTI_EINT0);
	NVIC_EnableIRQ(EINT0_IRQn);
}

void cfgTimer0(){
	TIM_TIMERCFG_Type cfgTimer0;
	TIM_MATCHCFG_Type cfgMatch1;

	cfgTimer0.PrescaleOption = TIM_PRESCALE_USVAL;
	cfgTimer0.PrescaleValue = 1000;

	cfgMatch1.MatchChannel = 1;
	cfgMatch1.IntOnMatch = DISABLE;
	cfgMatch1.ResetOnMatch = ENABLE;
	cfgMatch1.StopOnMatch = DISABLE;
	cfgMatch1.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
	cfgMatch1.MatchValue = 500 - 1;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &cfgTimer0);
	TIM_ConfigMatch(LPC_TIM0, &cfgMatch1);
	TIM_Cmd(LPC_TIM0, ENABLE);
}

void cfgDMA(){
	GPDMA_Channel_CFG_Type cfgADC = {0};
	GPDMA_Channel_CFG_Type cfgM2M = {0};
	GPDMA_Channel_CFG_Type cfgDAC = {0};
	GPDMA_Channel_CFG_Type cfgDAC_WAVE = {0};

	GPDMA_LLI_Type cfgADC_LLI = {0};
	GPDMA_LLI_Type cfgDAC_LLI = {0};
	GPDMA_LLI_Type cfgDAC_WAVE_LLI = {0};

	cfgADC_LLI.SrcAddr = (uint32_t)& LPC_ADC->ADDR0;
	cfgADC_LLI.DstAddr = (uint32_t) adc_samples;
	cfgADC_LLI.NextLLI = (uint32_t) &cfgADC_LLI;
	cfgADC_LLI.Control = (TRANSFER_SIZE^0xFFF<<0)|(2<<18)|(2<<21)|(1<<27)&~(1<<26);

	cfgADC.ChannelNum = 0;
	cfgADC.SrcMemAddr = 0;
	cfgADC.DstMemAddr = (uint32_t)adc_samples;
	cfgADC.TransferSize = TRANSFER_SIZE;
	cfgADC.TransferType = GPDMA_TRANSFERTYPE_P2M;
	cfgADC.SrcConn = GPDMA_CONN_ADC;
	cfgADC.DstConn = 0;
	cfgADC.DMALLI = 0;

	cfgM2M.ChannelNum = 7;
	cfgM2M.SrcMemAddr = (uint32_t)SRAM0;
	cfgM2M.DstMemAddr = (uint32_t)SRAM0_HALF;
	cfgM2M.TransferSize = TRANSFER_SIZE;
	cfgM2M.TransferType = GPDMA_TRANSFERTYPE_M2M;
    cfgM2M.TransferWidth = 1;
	cfgM2M.SrcConn = 0;
	cfgM2M.DstConn = 0;
	cfgM2M.DMALLI = 0;

	cfgDAC_LLI.SrcAddr = (uint32_t)& LPC_DAC->DACR;
	cfgDAC_LLI.DstAddr = (uint32_t) adc_samples;
	cfgDAC_LLI.NextLLI = (uint32_t) &cfgDAC_LLI;
	cfgDAC_LLI.Control = (TRANSFER_SIZE<<0)|(2<<18)|(2<<21)|(1<<26)&~(1<<27);

	cfgDAC.ChannelNum = 1;
	cfgDAC.SrcMemAddr = (uint32_t)adc_samples;
	cfgDAC.DstMemAddr = 0;
	cfgDAC.TransferSize = TRANSFER_SIZE;
	cfgDAC.SrcConn = 0;
	cfgDAC.DstConn = GPDMA_CONN_DAC;
	cfgDAC.DMALLI = (uint32_t)&cfgDAC_LLI;

	cfgDAC_WAVE_LLI.SrcAddr = (uint32_t)dac_wave;
	cfgDAC_WAVE_LLI.DstAddr = (uint32_t)LPC_DAC->DACR;
	cfgDAC_WAVE_LLI.NextLLI = (uint32_t)&cfgDAC_WAVE_LLI;
	cfgDAC_WAVE_LLI.Control = (NUM_SAMPLES<<0)|(2<<18)|(2<<21)|(1<<26)&~(1<<27);

	cfgDAC_WAVE.ChannelNum = 2;
	cfgDAC_WAVE.SrcMemAddr = (uint32_t)dac_wave;
	cfgDAC_WAVE.DstMemAddr = 0;
	cfgDAC_WAVE.TransferSize = NUM_SAMPLES;
	cfgDAC_WAVE.TransferType = GPDMA_TRANSFERTYPE_M2P;
    cfgDAC_WAVE.TransferWidth = 1;
	cfgDAC_WAVE.SrcConn = 0;
	cfgDAC_WAVE.DstConn = GPDMA_CONN_DAC;
	cfgDAC_WAVE.DMALLI = (uint32_t)&cfgDAC_WAVE_LLI;

	GPDMA_Setup(&cfgADC);
	GPDMA_Setup(&cfgM2M);
	GPDMA_Setup(&cfgDAC);
	GPDMA_Setup(&cfgDAC_WAVE);

}


int main(){
	cfgPBC();
	cfgEINT();
	cfgADC();
	cfgDAC();
	cfgTimer0();
	cfgDMA();
	generate_wave();
	while(1);
}

void generate_wave(){
	uint32_t quarter = NUM_SAMPLES/4;
	for(int i=0; i>quarter; i++){
		dac_wave[i] = DAC_MID + (i*DAC_MID/quarter);
		dac_wave[i+quarter] = DAC_MAX - (i*DAC_MID/quarter);
		dac_wave[i+2*quarter] = DAC_MID - (i*DAC_MID/quarter);
		dac_wave[i+3*quarter] = (i*DAC_MID/quarter);
	}
}

void  EINT0_IRQHandler(){
	static uint32_t count = 0;
	count = (count+1)%6;
	switch(count){
	case 1: // Modo punteros 
		GPDMA_ChannelCmd(2, DISABLE);
		adc_pointer_mode = 1;
		ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
		ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT01);
		break;
	case 2: // Modo DMA
		adc_pointer_mode = 0;
		ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, DISABLE);
		NVIC_DisableIRQ(ADC_IRQn);
		GPDMA_ChannelCmd(0, ENABLE);
		break;
	case 3:
		LPC_ADC->ADCR &= ~(7<<4);
		GPDMA_ChannelCmd(0, DISABLE);
		GPDMA_ChannelCmd(7, ENABLE);
		break;
	case 4:
		GPDMA_ChannelCmd(7, DISABLE);
		GPDMA_ChannelCmd(1, ENABLE);
		break;
	case 5:
		GPDMA_ChannelCmd(1, DISABLE);
		GPDMA_ChannelCmd(2, ENABLE);
        break;
    default:
        break;
	}
	EXTI_ClearEXTIFlag(EXTI_EINT0);
}

void ADC_IRQHandler(){
    uint32_t result = 0;
	if(ADC_ChannelGetStatus(LPC_ADC, 0, ADC_DATA_DONE)){
		result = ADC_ChannelGetData(LPC_ADC, 0) & 0x3FF;
	}

	if(adc_pointer_mode){
		adc_samples[sample_index] = result;
		sample_index = (sample_index+1)%TRANSFER_SIZE;
	}
}
