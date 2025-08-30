/*
 * Considerando pulsadores normalmente abiertos conectados en un extremo a
 * masa y en el otro directamente a las entradas P0.0 y p0.1. Realizar un
 * programa que identifique en una variable cual o cuales pulsadores han sido
 * presionados. Las identificaciones posibles a implementar en esta variable van a
 * ser "ninguno", "pulsador 1", "pulsador 2", "pulsador 1 y 2".
 */

// ninguno -> P0.2 encendido
// pulsador 1 -> P0.3 encendido
// pulsador 2 -> P0.4 encendido
// pulsador 1 y 2 -> P0.5 encendido

#include "LPC17xx.h"
#include <stdint.h>

void configPorts(void);

int main(void){
	configPorts();
	while(1){
		uint32_t p1 =  LPC_GPIO0->FIOPIN & 0x01;
		uint32_t p2 =  LPC_GPIO0->FIOPIN & 0x02;
		LPC_GPIO0->FIOCLR = (15<<2);
		if(p1&p2){
			LPC_GPIO0->FIOSET = (1<<2);
		}else if(!p1){
			LPC_GPIO0->FIOSET = (1<<3);
		}else if(!p2){
			LPC_GPIO0->FIOSET = (1<<4);
		}else{
			LPC_GPIO0->FIOSET = (1<<5);
		}
	}
	return 0;
}
// entradas: P0.0 y P0.1 salidas: P0.2, P0.3, P0.4, P0.5
void configPorts(){
	LPC_PINCON->PINSEL0 &= ~(0x3FF); 
	LPC_PINCON->PINMODE0 &= ~(0x0F);   // limpio bits de P0.0 y P0.1 → quedan en pull-up
	LPC_GPIO0->FIODIR |= (0x0F << 2);			// XX11 1100
}