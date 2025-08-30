/*
Escriba un programa que identifique la presión de un pulsador conectado a un pin GPIO configurado
como entrada (P0.0). Utiliza otro pin de salida para encender un LED solo mientras el botón está presionado (P0.22).
*/
#include "LPC17xx.h"
#include <stdint.h>

#define BUTTON (1 << 0)
#define LED (1 << 22)
#define DEBOUNCE_TIME 10000

void configGPIO(void){
    LPC_PINCON->PINSEL0 &= ~(3);
    LPC_PINCON->PINSEL1 &= ~(3 << 12); // (22-16)*2 = 12
    // por defecto, pull-up -> alto cuando esta presionado
    LPC_GPIO0->FIODIR |= LED;
    // por defecto, sin mask
}

void delay(uint32_t count){
    for(uint32_t i = 0; i < count; i++);
}

void main(void){
    configGPIO();
    while(1){
        if(LPC_GPIO0->FIOPIN & BUTTON){
            LPC_GPIO0->FIOCLR = LED;
        }else{
            LPC_GPIO0->FIOSET = LED;
        }
        delay(DEBOUNCE_TIME);
    }
    return 0;
}