/*
 * Una famosa empresa de calzados a incorporado a sus zapatillas 10 luces leds
 * comandadas por un microcontrolador LPC1769 y ha pedido a su grupo de
 * ingenieros que diseñen 2 secuencias de luces que cada cierto tiempo se vayan
 * intercalando (secuencia A - secuencia B- secuencia A- ... ). Como todavía no se
 * ha definido la frecuencia a la cual va a funcionar el CPU del microcontrolador,
 * las funciones de retardos que se incorporen deben tener como parámetros de
 * entrada variables que permitan modificar el tiempo de retardo que se vaya a
 * utilizar finalmente. Se pide escribir el código que resuelva este pedido,
 * considerando que los leds se encuentran conectados en los puertos P0,0 al
 * P0.9.
 */

#include "LPC17xx.h"
#include <stdint.h>

#include <cr_section_macros.h>

#define DELAY 10000000

void configPort();
void delay (uint32_t delayTime);
void secuencia1();
void secuencia2();

int main(void) {
	configPort();
    while(1) {
    	secuencia1();
    	secuencia2();
    }
    return 0 ;
}
// P0.0 a P0.9 como salida
void configPort(){
    // 0000 0000 0000 0011 1111 1111 1111 1111
	LPC_PINCON->PINSEL0 &= ~(0x3FFFF); // pone los pines 0 a 10 del puerto 0 como GPIO
    // 00000000 00000000 00000001 11111111
	LPC_GPIO0->FIODIR |= (0x1FF); //pone los pines 0 a 10 del puerto 0 como salida
	LPC_GPIO0->FIOMASK &= ~(0x1FF);
}

void secuencia1(){
	LPC_GPIO0->FIOCLR = (0x1FF<<0);
	for(int i=0; i<10; i++){
		LPC_GPIO0->FIOSET = (1<<i);
		delay(DELAY);
		LPC_GPIO0->FIOCLR = (1<<i);
	}
}

void secuencia2(){
	LPC_GPIO0->FIOCLR = (0x1FF<<0);
	for(int i=10; i>0; i--){
		LPC_GPIO0->FIOSET = (1<<i);
		delay(DELAY);
		LPC_GPIO0->FIOCLR = (1<<i);
	}
}

void delay(uint32_t delayTime){
	for(uint32_t i=0; i< delayTime; i++);
}