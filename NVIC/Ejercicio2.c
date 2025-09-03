/*
Realizar un programa que configure el puerto P0.0 y P2.0 para que
provoquen una interrupción por flanco de subida. Si la interrupción es
por P0.0 guardar el valor binario 100111 en la variable "auxiliar", si es
por P2.0 guardar el valor binario 111001011010110.
*/
#include "LPC17xx.h"
#include <stdint.h>

volatile uint32_t auxiliar = 0;
void configPorts(void);
void configNVIC(void);

int main(void){
    configPorts();
    configNVIC();
    while(1);
}

void configPorts(){
    LPC_PINCON->PINSEL0 &= ~(0x3); // P0.0 como GPIO
    LPC_PINCON->PINSEL4 &= ~(0x3); // P2.0 como GPIO
    // dejamos la pull ups por defecto
    LPC_GPIO0->FIODIR |= (1<<0); // P0.0 como salida
    LPC_GPIO2->FIODIR &= ~(1<<0); // P2.0 como entrada
}

void configNVIC(void){
    LPC_GPIOINT->IO0IntEnR |= (1<<0); // Habilitar interrupción por flanco de subida en P0.0
    LPC_GPIOINT->IO2IntEnR |= (1<<0); // Habilitar interrupción por flanco de subida en P2.0
    LPC_GPIOINT->IO0IntClr |= (1<<0); // Limpiar interrupciones
    LPC_GPIOINT->IO2IntClr |= (1<<0); // Limpiar interrupciones
    NVIC_SetPriority(EINT3_IRQn, 1); // Establecer prioridad de la interrupción externa 3
    NVIC_EnableIRQ(EINT3_IRQn); // Habilitar la interrupción externa 3
}

void EINT3_IRQHandler(void){
    if(LPC_GPIOINT->IntStatus & (1<<0)){
        if(LPC_GPIOINT->IO0IntStatR & (1<<0)){
            auxiliar = 0b100111;
        }
    } else if(LPC_GPIOINT->IntStatus & (4<<1)){
        if(LPC_GPIOINT->IO2IntStatR & (1<<0)){
            auxiliar = 0b111001011010110;
        }
    }
}