/*
Escriba un programa que lea el estado de todos los pines disponibles del Puerto 0 y cuente cuántos de
ellos están en un nivel alto (1 lógico). El resultado debe mostrarse en binario utilizando 5 LEDs
conectados a los pines menos significativos del Puerto 2.
*/
#include "LPC17xx.h"
#include <stdint.h>

#define INPUTS 0xFFFFFFFF
#define OUTPUTS 0x0000001F
#define DELAY_TIME 1000000

void configGPIO(void);
uint8_t getInput(void);
void showResult(uint8_t result);
void delay(uint32_t count);

int main(void){
    configGPIO();
    while(1){
        uint8_t input = getInput();
        delay(DELAY_TIME);
        showResult(input);
    }
    return 0;
}

void configGPIO(){
    LPC_PINCON->PINSEL0 &= ~(0xFFFFFFFF); // All GPIO
    LPC_PINCON->PINSEL4 &= ~(0x3FF);
    LPC_GPIO0->FIODIR &= ~INPUTS; // Set P0.0-P0.31 as input
    LPC_GPIO2->FIODIR |= OUTPUTS; // Set P2.0-P2.4 as output
    // mode and mask by default
}

uint8_t getInput(void){
    uint8_t count = 0;
    for(int i = 0; i < 32; i++){
        if(LPC_GPIO0->FIOPIN & (1 << i)){
            count++;
        }
    }
    return count;
}

void showResult(uint8_t result){
    LPC_GPIO2->FIOCLR = OUTPUTS;
    LPC_GPIO2->FIOSET = (result & 0x1F);
}

void delay(uint32_t count){
    for(uint32_t i = 0; i < count; i++);
}