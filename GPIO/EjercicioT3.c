/*
Escriba un programa que alterne entre dos o más secuencias de colores en el LED RGB. El tiempo de
duración de cada secuencia debe ser considerablemente mayor que el retardo entre los colores dentro
de la misma secuencia.
*/

#include "LPC17xx.h"
#include <stdint.h>
#include <cr_section_macros.h>

#define RED (1 << 22) // P0.22
#define GREEN (1 << 25) // P3.25
#define BLUE (1 << 26) //P3.26
// amarillo: rojo & verde
#define YELLOW (RED | GREEN)
// magenta: rojo & azul
#define MAGENTA (RED | BLUE)
// blanco: rojo & verde & azul
#define WHITE (RED | GREEN | BLUE) 
#define ON 10000000
#define OFF 10000

void configPorts(void);
void secuence_A(void);
void secuence_B(void);
void turnLed(uint32_t color);
void delay(int delayTime);

int main(){
	configPorts();
	while(1){
		secuence_A();
		secuence_B();
	}
    return 0;
}

void configPorts(){
	LPC_PINCON->PINSEL1 &= ~(3 << 12);
	LPC_PINCON->PINSEL7 &= ~((3 << 18) | (3 << 20)); // 0000 0000 0011 1100 0000 0000 0000 0000
	LPC_GPIO0->FIODIR |= (1<<22);
	LPC_GPIO3->FIODIR |= (3<<25);
}

// secuencia_A: rojo -> verde -> amarillo
void secuence_A(void){
    turnLed(RED);
    turnLed(GREEN);
    turnLed(YELLOW);
}

// secuencia_B: azul -> magenta -> blanco
void secuence_B(void){
    turnLed(BLUE);
    turnLed(MAGENTA);
    turnLed(WHITE);
}

void turnLed(uint32_t color){
    LPC_GPIO->FIOCLR = color;
    delay(ON);
    LPC_GPIO->FIOSET = color;
    delay(OFF);
}

void delay(int delayTime){
	for(int i=0; i<delayTime; i++){}
}