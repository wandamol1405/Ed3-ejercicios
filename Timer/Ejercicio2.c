/*
Escribir el código que configure el timer0 para cumplir con las
especificaciones dadas en la figura adjunta. (Pag 510 Figura 115 del
manual de usuario del LPC1769). Considerar una frecuencia de cclk de
100 Mhz y una división de reloj de periférico de 2.
*/

#include "LPC17xx.h"
#include "lpc17xx_timer.h"

void cfgTimer(){
	TIM_TIMERCFG_Type cfgTimerMode;
	TIM_MATCHCFG_Type cfgTimerMatch;
	
	cfgTimerMode.PrescaleOption = TIM_PRECALE_TICKS;
	cfgTimerMode.PrescaleValue = 2 - 1;
	cfgTimerMatch.MatchChannel = 0;
	cfgTimerMatch.MatchValue = 6 - 1;
	cfgTimerMatch.IntOnMatch = ENABLE;
	cfgTimerMatch.ResetOnMatch = ENABLE;
	cfgTimerMatch.StopOnMatch = DISABLE;
	cfgTimerMatch.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &cfgTimerMode);
	TIM_ConfigMatch(LPC_TIM0, &cfgTimerMatch);
	TIM_Cmd(LPC_TIM0, ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);
}

int main(){
	cfgTimer();
	while(1);
}

void TIMER0_IRQHandler(void){
	if(TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)){
		// TODO
	}
	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
}
