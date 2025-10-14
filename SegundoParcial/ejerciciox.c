/*
Utilizando CMSIS escriba y comente un código que genere una onda del tipo 
trapezoidal a la salida del DAC como se muestra en la figura. Para ello el DAC 
comenzará, a partir de cero, a incrementar su valor de a un bits hasta llegar a un 
valor máximo que se mantendrá un tiempo dado y luego decrementará de a un bits 
hasta volver a cero nuevamente. Los controles del valor máximo y los tiempos de 
duración de los escalones de subida y bajada están organizados en la posición de 
memoria 0x10004000 de la siguiente forma: 
	a) bits 0 a 7: valor máximo a alcanzar por el DAC. 
	b) bits 8 a 15: valor a utilizar en una función de demora que define el tiempo 
	   que se mantiene el valor máximo. 
	c) bits 16 a 23: valor a utilizar en una función de demora para definir el tiempo 
	   que se mantiene cada incremento de 1 bits en la subida. 
	d) bits 24 a 31: valor a utilizar en una función de demora para definir el tiempo 
   	   que se mantiene cada decremento de 1 bits en bajada.
*/
#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"

#define CONTROL_ADDR 0x10004000
#define WAVE_UP_ADDR    0x20080000
#define WAVE_DOWN_ADDR  0x2008C000

volatile uint32_t *control_value = (uint32_t *)CONTROL_ADDR;
volatile uint16_t *wave_up = (uint16_t *)WAVE_UP_ADDR;
volatile uint16_t *wave_down = (uint16_t *)WAVE_DOWN_ADDR;

uint8_t max_value;
uint8_t max_duration;
uint8_t inc_duration;
uint8_t dec_duration;

void read_control_values(void) {
    uint32_t val = *control_value;
    max_value    = (val >> 0) & 0xFF;
    max_duration = (val >> 8) & 0xFF;
    inc_duration = (val >> 16) & 0xFF;
    dec_duration = (val >> 24) & 0xFF;
}


void cfgPCB(){
	PINSEL_CFG_Type pin;

	// Configuracion de la salida analogica dle DAC
	pin.Portnum = 0;
	pin.Pinnum = 26;
	pin.Funcnum = 2;
	pin.Pinmode = PINSEL_PINMODE_TRISTATE;
	pin.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&pin);
}

void cfgDAC(){
	DAC_Init(LPC_DAC);
	DAC_CONVERTER_CFG_Type dac;
	dac.DBLBUF_ENA = DISABLE;
	dac.CNT_ENA = ENABLE;
	dac.DMA_ENA = ENABLE;
	DAC_ConfigDAConverterControl(LPC_DAC, &dac);
	DAC_SetDMATimeOut(inc_duration);
}

void cfgDMA(){
	GPDMA_Init();
	GPDMA_LLI_Type list;
	list.SrcAddr = (uint32_t)wave_up;
	list.DstAddr = (uint32_t)LPC_DAC->DACR;
	list.NextLLI = (uint32_t)&list;
	list.Control = (max_value)|(1<<18)|(2<<21)|(1<<26)|(1<<31);
	
	GPDMA_Channel_CFG_Type channel;
	channel.ChannelNum = 0;
	channel.SrcMemAddr = list.SrcAddr;
	channel.DstMemAddr = 0;
	channel.TransferSize = max_value;
	channel.TransferType = GPDMA_TRANSFERTYPE_M2P;
	channel.SrcConn = 0;
	channel.DstConn = GPDMA_CONN_DAC;
	channel.DMALLI = (uint32_t)&list;
	GPDMA_Setup(&channel);
	
	GPDMA_LLI_Type list2;
	list2.SrcAddr = (uint16_t)wave_down;
	list2.DstAddr = (uint32_t)LPC_DAC->DACR;
	list2.NextLLI = (uint32_t)&list2;
	list2.Control = (max_value)|(1<<18)|(2<<21)|(1<<26)|(1<<31);
	
	channel.ChannelNum = 1;
	channel.SrcMemAddr = list2.SrcAddr;
	channel.DMALLI = (uint32_t)&list2;
	GPDMA_Setup(&channel);
	NVIC_EnableIRQ(DMA_IRQn);
}

void wave_generator(void) {
    for(int i = 0; i <= max_value; i++)
        wave_up[i] = (i << 6);          // Subida
    for(int i = max_value; i >= 0; i--)
        wave_down[max_value - i] = (i << 6);  // Bajada
}

int main(){
	read_control_values();
	wave_generator();
	cfgPCB();
	cfgDAC();
	cfgDMA();
	GPDMA_ChannelCmd(0, ENABLE);
	while(1);
}

void DMA_IRQHandler(){
	if(GPDMA_IntGetStatus(GPDMA_STAT_INT, 0)==SET){
		if(GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 0)==SET){
			GPDMA_ChannelCmd(0, DISABLE);
			DAC_SetDMATimeOut(max_duration);
			SysTick_Config(SystemCoreClock / 1000 * max_duration); // tiempo relativo
			NVIC_EnableIRQ(SysTick_IRQn);
		}
		GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 0);
	}
	if(GPDMA_IntGetStatus(GPDMA_STAT_INT, 1)==SET){
		if(GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 1)==SET){
			GPDMA_ChannelCmd(1, DISABLE);
			DAC_SetDMATimeOut(inc_duration);
			
		}
		GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 1);
	}
	NVIC_ClearIntPending(DMA_IRQn);
}

void SysTick_Handler(){
	NVIC_DisableIRQ(SysTick_IRQn);
	GPDMA_ChannelCmd(1, ENABLE);
	DAC_SetDMATimeOut(LPC_DAC, dec_duration);
	NVIC_ClearPendingIRQ(SysTick_IRQn);
}
