/*
1.- Configure el Systick Timer de modo que genere una forma de onda
llamada PWM. Esta señal debe ser extraida por el pin P0.22 para que
controle la intensidad de brillo del led. El periodo de la señal debe ser
de 10ms con un duty cycle de 10%. Configure la interrupción externa
EINT0 de modo que cada vez que se entre en una rutina de
interrupción externa el duty cycle incremente en un 10% (1ms). Esto se
repite hasta llegar al 100%, luego, si se entra nuevamente a la
interrupción externa, el duty cycle volverá al 10%.
2.- Modificar los niveles de prioridad para que la interrupción por systick
tenga mayor prioridad que la interrupción externa.
*/

#include "LPC17xx.h"
#include <stdint.h>

#define LOAD_10MS 0xF423F  // Valor de recarga para 10ms

static uint32_t time_load = LOAD_10MS / 10; // Variable para almacenar el valor de recarga
static uint8_t duty_cycle = 10; // Duty cycle en porcentaje (10% inicial)
static uint8_t pwm_state = 0; // 0: ON, 1: OFF

void cfgGPIO(void);         // Configura los pines GPIO
void cfgEINT(void);         // Configura las interrupciones externas
void cfgSysTick(uint32_t load); // Configura el SysTick

int main(void){
    cfgGPIO();               // Configura los pines GPIO
    cfgEINT();               // Configura las interrupciones externas
    cfgSysTick(LOAD_10MS);   // Configura el SysTick con 10ms
    while(1){
        // Bucle principal vacío, todo se maneja por interrupciones
    }
    return 0;
}

void cfgGPIO(void){
    LPC_PINCON->PINSEL1 &= ~(3<<12); // P0.22 como GPIO
    LPC_GPIO0->FIODIR |= (1<<22);    // P0.22 como salida
}

void cfgEINT(){
    LPC_PINCON-> PINSEL4 |= (1<<20); // P2.10 como EINT0
    LPC_SC->EXTMODE |= (1<<0);       // EINT0 por flanco
    LPC_SC->EXTPOLAR |= (1<<0);         // EINT0 por flanco de subida
    LPC_SC->EXTINT |= (1<<0);        // Limpia interrupción pendiente
    NVIC_SetPriority(EINT0_IRQn, 2); // Prioridad 2 para EINT0
    NVIC_EnableIRQ(EINT0_IRQn);      // Habilita EINT0
}

void cfgSysTick(uint32_t load){
    SysTick->LOAD = load;  // Valor de recarga
    SysTick->VAL = 0; // Limpia el valor actual
    SysTick->CTRL = 0x07; // Habilitar SysTick con interrupciones y reloj del sistema
    NVIC_SetPriority(SysTick_IRQn, 1); // Prioridad 1 para SysTick
}

void EINT0_IRQHandler(void){
    duty_cycle = (duty_cycle + 10) % 100; // Incrementa el duty cycle en 10%

    time_load = (LOAD_10MS * duty_cycle) / 100; // Actualiza el valor de recarga según duty cycle
    LPC_SC->EXTINT |= (1<<0); // Limpia el flag de interrupción
}

void SysTick_Handler(void){
    if (pwm_state == 0) {
        LPC_GPIO0->FIOSET = (1<<22); // LED ON
        SysTick->LOAD = (duty_cycle * (LOAD_10MS / 10)) - 1; // ON time
        pwm_state = 1;
    } else {
        LPC_GPIO0->FIOCLR = (1<<22); // LED OFF
        SysTick->LOAD = ((10 - duty_cycle) * (LOAD_10MS / 10)) - 1; // OFF time
        pwm_state = 0;
    }
    SysTick->VAL = 0; // Limpia el valor actual
}