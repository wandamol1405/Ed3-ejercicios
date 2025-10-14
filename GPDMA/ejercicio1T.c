/*
Consigna: Desarrolle un programa que utilice el DMA para generar un patrón de Modulación por Ancho de
Pulso (PWM) en nueve pines de salida simultáneamente, cada uno con un ciclo de trabajo diferente.
Lógica:
• Cree un array en la memoria que contenga el patrón de salida digital deseado (donde cada bit
representa el estado de un pin de salida en ese momento).
• El patrón en el array debe simular 9 ondas PWM con la misma frecuencia, pero con ciclos de trabajo
incrementales: 10%, 20%, 30%, ..., hasta 90%.
Condiciones:
• El request de salida del DMA debe ser activado por un evento Match del Timer (e.g., MR0.0).
• La transferencia debe ser continua (utilizando el LLI en modo circular o configurando el TransferSize
para el bucle).
*/

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_gpdma.h"

#define OUTPUTS 10
#define PRESCALE_VALUE 1000
#define MATCH_VALUE 1000 - 1

uint16_t states[OUTPUTS] = {
		0x1FF, //111111111,
		0x1FE, //b111111110,
		0x1FC, //b111111100,
		0x1F8, //b111111000,
		0x1F0, //b111110000,
		0x1E0, //b111100000,
		0x1C0, //b111000000,
		0x180, //b110000000,
		0x100, //b100000000,
		0x000  //b000000000
};

void cfgPin(){
	LPC_PINCON->PINSEL0 &= ~(0x3FFFF);
	LPC_GPIO0->FIODIRL |= (0x1FF);
}

void cfgTimer(){
	TIM_TIMERCFG_Type timer;

	timer.PrescaleOption = TIM_PRESCALE_USVAL;
	timer.PrescaleValue = PRESCALE_VALUE;

	TIM_MATCHCFG_Type matcher;
	matcher.MatchChannel = 0;
	matcher.IntOnMatch = DISABLE; // A REVISAR
	matcher.ResetOnMatch = ENABLE;
	matcher.StopOnMatch = DISABLE;
	matcher.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	matcher.MatchValue = MATCH_VALUE;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timer);
	TIM_ConfigMatch(LPC_TIM0, &matcher);
	TIM_Cmd(LPC_TIM0, ENABLE);
}

void cfgDMA(){
	GPDMA_Init();
	GPDMA_LLI_Type list;
	list.SrcAddr = (uint32_t)states[0];
	list.DstAddr = (uint32_t)&(LPC_GPIO0->FIOPINL);
	list.NextLLI = (uint32_t)&list;
	list.Control = (OUTPUTS)|(1<<18)|(1<<21)|(1<<26);

	GPDMA_Channel_CFG_Type channel;
	channel.ChannelNum = 0;
	channel.SrcMemAddr = list.SrcAddr;
	channel.DstMemAddr = list.DstAddr;
	channel.TransferSize = OUTPUTS;
	channel.TransferType = GPDMA_TRANSFERTYPE_M2P;
	channel.SrcConn = 0;
	channel.DstConn = GPDMA_CONN_MAT0_0;
	channel.DMALLI = (uint32_t)&list;

	GPDMA_Setup(&channel);
	GPDMA_ChannelCmd(0, ENABLE);
}

int main(){
	cfgPin();
	cfgTimer();
	cfgDMA();
}
