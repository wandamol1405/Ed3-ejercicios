/*
Escribir un programa en C que permita sacar por los pines P1.16 al
P1.23 el equivalente en ascii de "a" si la interrupción se ha realizado
por interrupción de EINT1 y no hay interrupción pendiente de EINT0.
Para el caso en el cual se produzca una interrupción por EINT1 y exista
una interrupción pendiente de EINT0 sacar por el puerto el equivalente
en ascii de "A". La interrupción de EINT1 es por el flanco de subida
producido por un pulsador identificado como "tecla a/A", mientras que la
interrupción EINT0 es por el nivel alto de un pulsador etiquetado como
"Activación de Mayusculas".
Nota: Valerse de la configuración de los niveles de prioridad para que la
pulsación conjunta de "Activación de Mayúsculas" con "tecla a/A" de
como resultado a la salida el equivalente en ascii "A".
*/
#include "LPC17xx.h"
#include <stdint.h>

void configGPIO(void);
void configEINT(void);

int main(void){
	configGPIO();
	configEINT();
	while(1){
	}
	return 0;
}

void configGPIO(){
    LPC_PINCON->PINSEL3 &= ~(0xFFFF); // 1111 1111 1111 1111
    LPC_GPIO1->FIODIR |= (0xFF << 16);   
}

void configEINT(){
    LPC_PINCON->PINSEL4 |= (1<<20)|(1<<22); // P2.10 y P2.11 como EINT0 y EINT1
    LPC_SC->EXTMODE &= ~(1<<0);
    LPC_SC->EXTMODE |= (1<<1); // EINT0 por nivel, EINT1 por flanco
    LPC_SC->EXTPOLAR |= (1<<0)|(1<<1); // EINT0 por nivel alto, EINT1 por flanco de subida
    LPC_SC->EXTINT |= 3;
    NVIC_SetPriority(EINT1_IRQn, 1);
    NVIC_SetPriority(EINT0_IRQn, 2);
    NVIC_EnableIRQ(EINT0_IRQn);
    NVIC_EnableIRQ(EINT1_IRQn);
}

void EINT0_IRQHandler(){
    // Código para manejar la interrupción EINT0
}

void EINT1_IRQHandler(){
    if(LPC_SC->EXTINT & (1<<0)){
        // Si hay una EINT0 pendiente, sacar una A
        LPC_GPIO1->FIOPIN |= 0x41 << 16;
    }else{
        // Si no hay una EINT0 pendiente, sacar una a
        LPC_GPIO1->FIOPIN |= 0x61 << 16;
    }
    LPC_SC->EXTINT |= 3; 
}