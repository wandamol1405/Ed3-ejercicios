/*
Escribir un programa para que por cada presión de un pulsador, la
frecuencia de parpadeo disminuya a la mitad debido a la modificación
del registro del Match 0. El pulsador debe producir una interrupción por
EINT2 con flanco descendente.
*/

#include "LPC17xx.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_exti.h"
#include <stdint.h>

uint32_t pulsations = 0;

void cfgTimer(){
	TIM_TIMERCFG_Type cfgTimerMode;
	TIM_MATCHCFG_Type cfgTimerMatch;

	cfgTimerMode.PrescaleOption = TIM_PRESCALE_USVAL;
	cfgTimerMode.PrescaleValue = 1000; // 1ms
	cfgTimerMatch.MatchChannel = 0;
	cfgTimerMatch.MatchValue = 1000 - 1;  // 1s
	cfgTimerMatch.IntOnMatch = ENABLE;
	cfgTimerMatch.ResetOnMatch = ENABLE;
	cfgTimerMatch.StopOnMatch = DISABLE;
	cfgTimerMatch.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	TIM_Init(LPC_TIM2, TIM_TIMER_MODE, &cfgTimerMode);
	TIM_ConfigMatch(LPC_TIM2, &cfgTimerMatch);
	TIM_Cmd(LPC_TIM2, ENABLE);
	NVIC_EnableIRQ(TIMER2_IRQn);
}

void cfgGPIO()
{
	PINSEL_CFG_Type cfgGPIO;

	cfgGPIO.Portnum = PINSEL_PORT_0;
	cfgGPIO.Pinnum = PINSEL_PIN_22;
	cfgGPIO.Funcnum = PINSEL_FUNC_0; // Funcion GPIO
	cfgGPIO.Pinmode = PINSEL_PINMODE_TRISTATE; // Sin resistencia
	cfgGPIO.OpenDrain = PINSEL_PINMODE_NORMAL;
	GPIO_SetDir(PINSEL_PORT_0, PINSEL_PIN_22, OUTPUT);
	PINSEL_ConfigPin(&cfgGPIO);
}

void cfgEINT2(){
	PINSEL_CFG_Type cfgPin;

	cfgPin.Portnum = PINSEL_PORT_2;
	cfgPin.Pinnum = PINSEL_PIN_12;
	cfgPin.Funcnum = PINSEL_FUNC_1; // Funcion EINT1
	cfgPin.Pinmode = PINSEL_PINMODE_PULLUP;
	cfgPin.OpenDrain = PINSEL_PINMODE_NORMAL;

	EXTI_SetMode(EXTI_EINT2, EXTI_MODE_EDGE_SENSITIVE);
	EXTI_SetPolarity(EXTI_EINT2, EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE);
	EXTI_ClearEXTIFlag(EXTI_EINT2);
	PINSEL_ConfigPin(&cfgPin);
	NVIC_EnableIRQ(EINT2_IRQn);
}

int main(){
	cfgGPIO();
	cfgEINT2();
	cfgTimer();
	while(1);
}

void TIMER2_IRQHandler(void){
	if(TIM_GetIntStatus(LPC_TIM2, TIM_MR0_INT)){
		LPC_GPIO0->FIOPIN ^= (1<<22); // Toggle LED
		TIM_ClearIntPending(LPC_TIM2, TIM_MR0_INT);
	}

}

void EINT1_IRQHandler(void){
	uint32_t new_match = (LPC_TIM2->PR+1)*2-1;
	if(new_match>0xFFFFFFFF){
		new_match = 1000;
	}
	LPC_TIM2->MR = new_match;
	EXT1_ClearEXTIFlag(EXTI_EINT2);
}
