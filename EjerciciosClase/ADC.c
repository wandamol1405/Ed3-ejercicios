/*
 * CONFIGURAR UNA INTERRUPCION DE GPIO. EL HANDLER TIENE QUE HACER PARPADEAR UN LED A UNA FRECUENCIA
 * EN LA PRIMERA INTERRUPCION Y A OTRA EN LA SEGUNDA INTERRUPCION
 */
#include "LPC17xx.h"
#include <stdint.h>

void cfgPCB(void);    // P0.22 led
void cfgExtInt(void); // P0.15 interrupcion GPIO -> por subida
void cfgSysTick(void);
void cfgADC(void);

uint16_t *buffer =(uint16_t *)0x2007c000; // puntero donde guardar los datos del ADC

int main(void){
	cfgGPIO();
	cfgExtInt();
	cfgSysTick();
	cfgADC();
	while(1){
	}
	return 0;
}
void cfgPCB(){
	// ADC0 al ADC3 -> 01 del P0.23 al P0.26
	// ADC4 y ADC5 -> 11 en P1.30 y P1.31
	// ADC6 y ADC7 -> 10 en P0.3 y P0.2 respectivamente
	
	LPC_PINCON->PINSEL0 |= (2 << 4); //MODIFICAR AL ADC0
	// PULL UPS POR DEFAULT
	// MASK DEFAULT
}

void cfgExtInt(){
	//TODO
}

void cfgADC(){
	LPC_SC->PCONP |= 1<<12;
	LPC_ADC->ADCR |= (1 << 8) | (1 << 21); //DIVIDIENDO EL CLOCK QUE VIENE DEL PERIFERICO -> TRABAJA A 12.5MHz
	// que arranque con la interrupcion de SysTick
	LPC_ADC->ADINTEN |= 1<<0; // PARA QUE INTERRUMPA EL CANAL 0
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
