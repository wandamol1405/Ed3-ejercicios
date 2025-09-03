/*
Realizar un programa que configure el puerto P2.0 y P2.1 para que
provoquen una interrupción por flanco de subida para el primer pin y
por flanco de bajada para el segundo. Cuando la interrupción sea por
P2.0 se enviará por el pin P0.0 la secuencia de bits 010011010. Si la
interrupción es por P2.1 se enviará por el pin P0.1 la secuencia
011100110. Las secuencias se envían únicamente cuando se produce
una interrupción, en caso contrario la salida de los pines tienen valores
1 lógicos. ¿que prioridad tienen configuradas por defecto estas
interrupciones?
-> Por defecto, las interrupciones GPIO tienen una prioridad 6
*/
#include "LPC17xx.h"
#include <stdint.h>

void configPorts(void);
void configNVIC(void);
void sendSequence(uint8_t pin);

int main(void){
    configPorts();
    configNVIC();
    while(1);
}

void configPorts(void){
    LPC_PINCON->PINSEL0 &= ~(0xF); // P0.0 y P0.1 como GPIO
    LPC_PINCON->PINSEL4 &= ~(0xF); // P2.0 y P2.1 como GPIO
    // P2.0 y P2.1 como entradas -> Pull ups activas
    // P0.0 y P0.1 como salidas -> Sin resistencias
    LPC_PINCON->PINMODE0 |= (0xA); // P0.0 y P0.1 sin resistencias
    LPC_GPIO0->FIODIR |= (0x3); // P0.0 y P0.1 como salidas
    LPC_GPIO2->FIODIR &= ~(0x3); // P2.0 y P2.1 como entradas
}

void configNVIC(void){
    LPC_GPIOINT->IO2IntEnR |= (1<<0); // Habilitar interrupción por flanco de subida en P2.0
    LPC_GPIOINT->IO2IntEnF |= (1<<1); // Habilitar interrupción por flanco de bajada en P2.1
    LPC_GPIOINT->IO2IntClr |= (1<<0) | (1<<1); // Limpiar interrupciones
    NVIC_SetPriority(EINT3_IRQn, 1); // Establecer prioridad de la interrupción externa 3
    NVIC_EnableIRQ(EINT3_IRQn); // Habilitar la interrupción externa 3
}

void EINT3_IRQHandler(void){
    if(LPC_GPIOINT->IntStatus & (2<<0)){
        if(LPC_GPIOINT->IO2IntStatR & (1<<0)){
            sendSequence(0);
        } else if (LPC_GPIOINT->IO2IntStatF & (1<<1)){
            sendSequence(1);
        }
    }
}

void sendSequence(uint8_t pin){
    if(!pin){
        LPC_GPIO0->FIOPIN |= (0x9A); // Enviar 010011010
    } else{
        LPC_GPIO0->FIOPIN |= (0xE6); // Enviar 011100110
    }
}