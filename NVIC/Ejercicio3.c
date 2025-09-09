/*
Configurar la interrupción externa EINT1 para que interrumpa por flanco
de bajada y la interrupción EINT2 para que interrumpa por flanco de
subida. En la interrupción por flanco de bajada configurar el systick para
desbordar cada 25 mseg, mientras que en la interrupción por flanco de
subida configurarlo para que desborde cada 60 mseg. Considerar que
EINT1 tiene mayor prioridad que EINT2.
*/
#include "LPC17xx.h"
#include <stdint.h>

#define SYSTICK_LOAD_VAL_25MS  (SystemCoreClock * 25 - 1)
#define SYSTICK_LOAD_VAL_60MS  (SystemCoreClock * 60 - 1)

void configPorts(void);
void configNVIC(void);
void configSysTick(uint32_t loadValue);

int main(void){
    configPorts();
    configNVIC();
    while(1);
}

void configPorts(void){
    LPC_PINCON->PINSEL4 &= ~(0xF); // P2.0 y P2.1 como GPIO
    LPC_PINCON->PINSEL4 |= (0x5<<11); // P2.10 como EINT1, P2.11 como EINT2
    LPC_GPIO2->FIODIR &= ~(0x3); // P2.0 y P2.1 como entrada
}

void configNVIC(){
    LPC_SC->EXTMODE |= (1<<1) | (1<<2); // EINT1 por flanco, EINT2 por flanco
    LPC_SC->EXTPOLAR &= ~(1<<1); // EINT1 por flanco de bajada
    LPC_SC->EXTPOLAR |= (1<<2); // EINT2 por flanco de subida
    LPC_SC->EXTINT |= (1<<1) | (1<<2); // Limpiar interrupciones
    NVIC_SetPriority(EINT1_IRQn, 1);
    NVIC_SetPriority(EINT2_IRQn, 2);
    NVIC_EnableIRQ(EINT1_IRQn);
    NVIC_EnableIRQ(EINT2_IRQn);
}

void configSysTick(uint32_t loadValue){
    SysTick->LOAD = loadValue;
    SysTick->VAL = 0;
    SysTick->CTRL = 0x07; // Habilitar SysTick con interrupciones y reloj del sistema
}

void EINT1_IRQHandler(void){
    configSysTick(SYSTICK_LOAD_VAL_25MS);
    LPC_SC->EXTINT |= (1<<1); // Limpiar interrupcion EINT1
}

void EINT2_IRQHandler(void){
    configSysTick(SYSTICK_LOAD_VAL_60MS);
    LPC_SC->EXTINT |= (1<<2); // Limpiar interrupcion EINT2
}
