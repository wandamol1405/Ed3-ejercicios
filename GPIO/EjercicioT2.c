// Escriba un programa que haga parpadear un color en el LED RGB integrado en la placa (enciende con 0)

#include "LPC17xx.h"
#include <stdint.h>
#include <cr_section_macros.h>

#define RED (1 << 22) // P0.22
#define GREEN (1 << 25) // P3.25
#define BLUE (1 << 26) //P3.26
#define ON 10000000
#define OFF 10000

void configPorts(void);
void delay(int delayTime);
void turnLed(uint32_t color);

int main(){
	configPorts();
	while(1){
		turnLed(RED);
		turnLed(GREEN);
		turnLed(BLUE);
	}
	return 0;
}

void turnLed(uint32_t color){
	LPC_GPIO->FIOCLR = color;
	delay(ON);
	LPC_GPIO->FIOSET = color;
	delay(OFF);
}

void configPorts(){
	LPC_PINCON->PINSEL1 &= ~(3 << 12);
	LPC_PINCON->PINSEL7 &= ~((3 << 18) | (3 << 20)); // 0000 0000 0011 1100 0000 0000 0000 0000
	LPC_GPIO0->FIODIR |= (1<<22);
	LPC_GPIO3->FIODIR |= (3<<25);
}

void delay(int delayTime){
	for(int i=0; i<delayTime; i++){}
}