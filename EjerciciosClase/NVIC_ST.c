/*
 * CONFIGURAR UNA INTERRUPCION DE GPIO. EL HANDLER TIENE QUE HACER PARPADEAR UN LED A UNA FRECUENCIA
 * EN LA PRIMERA INTERRUPCION Y A OTRA EN LA SEGUNDA INTERRUPCION
 */
#include "LPC17xx.h"
#include <stdint.h>

static uint8_t inte=0;
static uint32_t time;
static uint32_t VAL1 = 0xFFFFFF;
static uint32_t VAL2 = 0x98967F;
void cfgGPIO(void);    // P0.22 led
void cfgIntGPIO(void); // P0.15 interrupcion GPIO -> por subida
void cfgSysTick(void);

int main(void){
	cfgGPIO();
	cfgIntGPIO();
	cfgSysTick();
	while(1){
	}
	return 0;
}
void cfgGPIO(){
	LPC_PINCON->PINSEL0 &= ~(0b11 << 30); //P0.15 - > INTERRUPCION
	LPC_PINCON->PINSEL1 &= ~(0b11 << 12); //P0.22 -> LED
	LPC_GPIO0->FIODIR |= (1<<22); //P0.15 -> ENTRADA
	LPC_GPIO0->FIODIR &= ~(1<<15); //P0.22 -> SALIDA
	// MASK DEFAULT
	return; //lo puse porque me dio ternura
}

void cfgIntGPIO(){
	LPC_GPIOINT->IO0IntEnR |= (1<<15);
	LPC_GPIOINT->IO0IntClr |= (1<<15);
	//NVIC_SetPriority(EINT3_IRQn, 1); // a chequear
	NVIC_EnableIRQ(EINT3_IRQn);
	return;
}

void cfgSysTick(){
	SysTick->VAL = 0;
	SysTick->CTRL = 0x07;
}

void EINT3_IRQHandler(void){ //definido en la libreria de CMSIS
	if(LPC_GPIOINT->IntStatus & (1<<0)){
			// 2. QUE PIN INTERRUMPE
		if(LPC_GPIOINT->IO0IntStatR & (1<<15)){
			inte++;
			if(inte%2){
				time = VAL1;
			}else{
				time = VAL2;
				}
			LPC_GPIOINT->IO0IntClr|=(1<<15);
		}
	}

}

void SysTick_Handler(){
	SysTick->LOAD = time;
	LPC_GPIO0->FIOPIN ^= (1<<22);
	SysTick->CTRL &= SysTick->CTRL; // parece que lo hace solo --> a chequear
}
