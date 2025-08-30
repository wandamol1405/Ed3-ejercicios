/*
 * En los pines P2.0 a P2.7 se encuentra conectado un display de 7 segmentos.
 * Utilizando la variable numDisplay [10] ={0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D,
 * 0x7D, 0x07, 0x7F, 0x67} que codifica los números del 0 a 9 para ser mostrados
 * en el display, realizar un programa que muestre indefinidamente la cuenta de 0
 * a 9 en dicho display.
 */

#include "LPC17xx.h"
#include <stdint.h>

#define DELAY 10000000

void showNum(void);
void delay(void);
void configPort(void);

int main(void){

	configPort();
	while(1){
		showNum();
	}
}

void configPort(){
	LPC_PINCON->PINSEL4 &= ~(0x3FFF); //0011 1111 1111 1111
	LPC_GPIO2->FIODIR |= (0x7F); //0000 0000 0111 1111
	LPC_GPIO2->FIOMASK &= ~(0x7F);
}

void showNum(){
	uint8_t numDisplay[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67};
	for(int i=0; i<10; i++){
		LPC_GPIO2->FIOPIN = numDisplay[i];
		delay(DELAY);
	}
}

void delay(uint32_t delayTime){
	for(uint32_t i=0; i<delayTime; i++);
}