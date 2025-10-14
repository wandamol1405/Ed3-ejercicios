/*
Utilizando el timer0, un dac, interrupciones y el driver del LPC1769,
escribir un codigo que permita generar una señal triangular
periodica simetrica que tenga el minimo periodo posible , la maxima
excursion de voltaje pico a pico posible y un minimo incremento
de señal posible por el DAC. Suponer una frecuencia de core cclk de
100MHz. El codigo debe estar debidamente comentado.
*/

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_gpdma.h"

#define MAX_VALUE 1024
#define BUFFER_SIZE MAX_VALUE*2
uint16_t wave[BUFFER_SIZE]={0};
volatile uint16_t index;

void cfgPCB(){
    PINSEL_CFG_Type pin = {0};
    pin.Portnum = 0;
    pin.Pinnum = 26;
    pin.Funcnum = 2;
    pin.Pinmode = PINSEL_PINMODE_TRISTATE;
    pin.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&pin);
}

void cfgTimer(){
	TIM_TIMERCFG_Type timer = {0};
	timer.PrescaleOption = TIM_PRESCALE_USVAL;
	timer.PrescaleValue = 0;

	TIM_MATCHCFG_Type matcher = {0};
	matcher.MatchChannel = 0;
	matcher.IntOnMatch = ENABLE;
	matcher.ResetOnMatch = ENABLE;
	matcher.StopOnMatch = DISABLE;
	matcher.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	matcher.MatchValue = 1;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timer);
	TIM_ConfigMatch(LPC_TIM0, &matcher);
	TIM_Cmd(LPC_TIM0, ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);
}

void cfgDAC(){
	DAC_Init(LPC_DAC);
	DAC_CONVERTER_CFG_Type dac = {0};
	dac.DBLBUF_ENA = DISABLE;
	dac.CNT_ENA = DISABLE;
	dac.DMA_ENA = ENABLE;
	DAC_ConfigDAConverterControl(LPC_DAC, &dac);
}

void cfgDMA(){
	GPDMA_Init();
	GPDMA_LLI_Type list = {0};
	list.SrcAddr = (uint32_t)wave;
	list.DstAddr = (uint32_t)LPC_DAC->DACR;
	list.NextLLI = (uint32_t)&list;
	list.Control = (BUFFER_SIZE)|(1<<18)|(2<<21)|(1<<26);

	GPDMA_Channel_CFG_Type channel = {0};
	channel.ChannelNum = 0;
	channel.SrcMemAddr = (uint32_t)wave;
	channel.DstMemAddr = 0;
	channel.TransferSize = BUFFER_SIZE;
	channel.SrcConn = 0;
	channel.DstConn = GPDMA_CONN_DAC;
	channel.DMALLI = (uint32_t)&list;

	GPDMA_Setup(&channel);
	GPDMA_ChannelCmd(0, ENABLE);
}

void generate_wave(){
	for(int i=0; i<MAX_VALUE; i++){ wave[i]=(i<<6);}
	for(int i=MAX_VALUE; i>=0; i--){ wave[MAX_VALUE-i]=(i<<6);}
}

int main(){
	generate_wave();
	cfgPCB();
	cfgTimer();
	cfgDAC();
	cfgDMA();
	while(1);
}

void TIMER0_IRQHandler(){
	if(TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)==SET){
		index = (index+1)%BUFFER_SIZE;
		DAC_UpdateValue(LPC_DAC, wave[index]);
		TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
		NVIC_ClearPendingIRQ(TIMER0_IRQn);
	}
}
