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

uint8_t mode = 0;

void cfgTimer(){
	TIM_TIMERCFG_Type cfgTimer;
	TIM_MATCHCFG_Type cfgMatch0;
	TIM_MATCHCFG_Type cfgMatch1;
	TIM_MATCHCFG_Type cfgMatch2;
	TIM_MATCHCFG_Type cfgMatch3;

	cfgTimer.PrescaleOption = TIM_PRESCALE_USVAL;
	cfgTimer.PrescaleValue = 1000;
	cfgMatch0.MatchChannel = 0;
	cfgMatch1.MatchChannel = 1;
	cfgMatch2.MatchChannel = 2;
	cfgMatch3.MatchChannel = 3;
	cfgMatch0.MatchValue = 5000;
	cfgMatch1.MatchValue = 10000;
	cfgMatch2.MatchValue = 15000;
	cfgMatch3.MatchValue = 20000;
	cfgMatch0.IntOnMatch = ENABLE;
	cfgMatch1.IntOnMatch = ENABLE;
	cfgMatch2.IntOnMatch = ENABLE;
	cfgMatch3.IntOnMatch = ENABLE;
	cfgMatch0.ResetOnMatch = DISABLE;
	cfgMatch1.ResetOnMatch = DISABLE;
	cfgMatch2.ResetOnMatch = DISABLE;
	cfgMatch3.ResetOnMatch = DISABLE;
	cfgMatch0.StopOnMatch = DISABLE;
	cfgMatch1.StopOnMatch = DISABLE;
	cfgMatch2.StopOnMatch = DISABLE;
	cfgMatch3.StopOnMatch = DISABLE;
	cfgMatch0.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	cfgMatch1.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	cfgMatch2.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	cfgMatch3.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &cfgTimer);
	TIM_ConfigMatch(LPC_TIM0, &cfgMatch0);
	TIM_ConfigMatch(LPC_TIM0, &cfgMatch1);
	TIM_ConfigMatch(LPC_TIM0, &cfgMatch2);
	TIM_ConfigMatch(LPC_TIM0, &cfgMatch3);
	TIM_Cmd(LPC_TIM0, ENABLE);
	NVIC_SetPriority(TIMER0_IRQn, 0);
	NVIC_EnableIRQ(TIMER0_IRQn);
}

void cfgGPIO()
{
	PINSEL_CFG_Type cfgGPIO0;
	PINSEL_CFG_Type cfgGPIO1;
	PINSEL_CFG_Type cfgGPIO2;
	PINSEL_CFG_Type cfgGPIO3;

	cfgGPIO0.Portnum = PINSEL_PORT_0;
	cfgGPIO1.Portnum = PINSEL_PORT_0;
	cfgGPIO2.Portnum = PINSEL_PORT_0;
	cfgGPIO3.Portnum = PINSEL_PORT_0;
	cfgGPIO0.Pinnum = PINSEL_PIN_0;
	cfgGPIO1.Pinnum = PINSEL_PIN_1;
	cfgGPIO2.Pinnum = PINSEL_PIN_2;
	cfgGPIO3.Pinnum = PINSEL_PIN_3;
	cfgGPIO0.Funcnum = PINSEL_FUNC_0;
	cfgGPIO1.Funcnum = PINSEL_FUNC_0;
	cfgGPIO2.Funcnum = PINSEL_FUNC_0;
	cfgGPIO3.Funcnum = PINSEL_FUNC_0;
	cfgGPIO0.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgGPIO1.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgGPIO2.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgGPIO3.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgGPIO0.OpenDrain = PINSEL_PINMODE_NORMAL;
	cfgGPIO1.OpenDrain = PINSEL_PINMODE_NORMAL;
	cfgGPIO2.OpenDrain = PINSEL_PINMODE_NORMAL;
	cfgGPIO3.OpenDrain = PINSEL_PINMODE_NORMAL;
	GPIO_SetDir(PINSEL_PORT_0, PINSEL_PIN_0, OUTPUT);
	GPIO_SetDir(PINSEL_PORT_0, PINSEL_PIN_1, OUTPUT);
	GPIO_SetDir(PINSEL_PORT_0, PINSEL_PIN_2, OUTPUT);
	GPIO_SetDir(PINSEL_PORT_0, PINSEL_PIN_3, OUTPUT);
	PINSEL_ConfigPin(&cfgGPIO0);
	PINSEL_ConfigPin(&cfgGPIO1);
	PINSEL_ConfigPin(&cfgGPIO2);
	PINSEL_ConfigPin(&cfgGPIO3);
}

void cfgEINT3(){
	PINSEL_CFG_Type cfgPin;

	cfgPin.Portnum = PINSEL_PORT_2;
	cfgPin.Pinnum = PINSEL_PIN_13;
	cfgPin.Funcnum = PINSEL_FUNC_1;
	cfgPin.Pinmode = PINSEL_PINMODE_PULLDOWN;
	cfgPin.OpenDrain = PINSEL_PINMODE_NORMAL;
	EXTI_SetMode(EXTI_EINT3, EXTI_MODE_EDGE_SENSITIVE);
	EXTI_SetPolarity(EXTI_EINT3, EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE);
	EXTI_ClearEXTIFlag(EXTI_EINT3);
	PINSEL_ConfigPin(&cfgPin);
	NVIC_SetPriority(EINT3_IRQn, 2);
	NVIC_EnableIRQ(EINT3_IRQn);

}

int main(){
	cfgGPIO();
	cfgEINT3();
	cfgTimer();
	while(1);
}

void TIMER0_IRQHandler(void){
	if(TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)){
		if(!mode){
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_0);
			GPIO_SetValue(PINSEL_PORT_0, PINSEL_PIN_1);
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_2);
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_3);
		}else{
			GPIO_SetValue(PINSEL_PORT_0, PINSEL_PIN_0);
			GPIO_SetValue(PINSEL_PORT_0, PINSEL_PIN_1);
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_2);
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_3);

		}
		LPC_TIM0->MR0 = 15000;
	}
	if(TIM_GetIntStatus(LPC_TIM0, TIM_MR1_INT)){
		if(!mode){
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_0);
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_1);
			GPIO_SetValue(PINSEL_PORT_0, PINSEL_PIN_2);
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_3);
		}else{
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_0);
			GPIO_SetValue(PINSEL_PORT_0, PINSEL_PIN_1);
			GPIO_SetValue(PINSEL_PORT_0, PINSEL_PIN_2);
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_3);
		}
		LPC_TIM0->MR1 = 15000;
	}
	if(TIM_GetIntStatus(LPC_TIM0, TIM_MR2_INT)){
		if(!mode){
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_0);
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_1);
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_2);
			GPIO_SetValue(PINSEL_PORT_0, PINSEL_PIN_3);
		}else{
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_0);
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_1);
			GPIO_SetValue(PINSEL_PORT_0, PINSEL_PIN_2);
			GPIO_SetValue(PINSEL_PORT_0, PINSEL_PIN_3);
		}
		LPC_TIM0->MR2 = 15000;
	}
	if(TIM_GetIntStatus(LPC_TIM0, TIM_MR3_INT)){
		if(!mode){
			GPIO_SetValue(PINSEL_PORT_0, PINSEL_PIN_0);
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_1);
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_2);
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_3);
		}else{
			GPIO_SetValue(PINSEL_PORT_0, PINSEL_PIN_0);
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_1);
			GPIO_ClearValue(PINSEL_PORT_0, PINSEL_PIN_2);
			GPIO_SetValue(PINSEL_PORT_0, PINSEL_PIN_3);
		}
		LPC_TIM0->MR3 = 15000;
	}

	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);

}

void EINT3_IRQHandler(void){
	if(LPC_SC->EXTINT & (1<<3)){
		mode ^= 1;
		EXTI_ClearEXTIFlag(EXTI_EINT3);
	}
}
