/*
EJERCICIO INTEGRADOR PARCIAL I

En la dirección de memoria 0x20080000 se tiene almacenado un valor de 32 bits
que representa cuatro formas de onda binarias de 8 bits cada una.
Utilizando los registros de configuración y el systick del microcontrolador LPC1769
generar por el pin P2.8 las formas de onda binarias en serie de 8 bits almacenadas en la
dirección anteriormente mencionada, y generar por el puerto P2 el promedio de la forma de
onda binaria seleccionada.
El pin asociado a EINT0 presenta una resistencia de pull-down externa.
Configurar la interrupción de dicho pin con prioridad 3, para que, cada vez que interrumpa,
termine la forma de onda actual y cambie a la siguiente (una vez que llega a la última,
debe volver a comenzar la primera).
El periodo de la señal debe ser establecido mediante la interrupción (prioridad 2)
del pin asociado a EINT1, el cual presenta una resistencia de pull-up. De manera que se pueda
cambiar el periodo de la señal entre 80[ms] (por defecto) y 160[ms]. Considerando un cclk de 65[MHz].
 */

// P2.8 -> SALIDA DE LA FORMA DE ONDA (GPIO)
// P2.0 a P2.7 -> SALIDA DEL PROMEDIO DE LA FORMA DE ONDA SELECCIONADA (GPIO)
// P2.10 -> EINT0 POR FLANCO DE SUBIDA Y PRIORIDAD 3 
// P2.11 -> EINT1 POR FLANCO DE BAJADA Y PRIORIDAD 2

// BUFFER CIERCULAR
// SetWave = (SetWave + 1)%NUM_WAVE;

#include "LPC17xx.h"
#include <stdint.h>

uint32_t load_80 = 0x9EB0F; // Valor de recarga para 80ms (65MHz)
uint32_t load_160 = 0x13D61F; // Valor de recarga para 160ms (65MHz)
#define NUM_WAVE 4      // Número de formas de onda
#define SIZE_WAVE 8     // Tamaño de cada forma de onda (8 bits)

uint32_t nTicks;        // Contador de ticks para alternar el periodo
static int wave_offset = 0; // Offset para seleccionar la forma de onda actual
static int setWave=0;       // Índice de la forma de onda actual

void cfgGPIO(void);         // Configura los pines GPIO
void cfgEINT(void);         // Configura las interrupciones externas
void cfgSysTick(void);      // Configura el SysTick
uint8_t average(void);      // Calcula el promedio de la forma de onda seleccionada

uint32_t *waveForm = (uint32_t *)0x20080000; // Apuntador a las formas de onda en memoria

int main(void){
    cfgGPIO();               // Configura los pines GPIO
    cfgEINT();               // Configura las interrupciones externas
    cfgSysTick(load_80);     // Configura el SysTick con 80ms por defecto
    while(1){
        // Bucle principal vacío, todo se maneja por interrupciones
    }
    return 0;
}

// Configura los pines GPIO
void cfgGPIO(){
    LPC_PINCON->PINSEL4 &= ~(0x3FFFF); // Configura P2.0-P2.8 como GPIO
    LPC_GPIO2->FIODIR |= (0x1FF);      // Configura P2.0-P2.8 como salida
}

// Configura las interrupciones externas EINT0 y EINT1
void cfgEINT(){
    LPC_PINCON->PINSEL4 |= (1<<20) | (1<<22); // P2.10 como EINT0, P2.11 como EINT1
    LPC_PINCON->PINMODE4 |= (3<<20);        // P2.10 con resistencia de pull-down
    LPC_SC->EXTMODE |= (3<<0);                // EINT0 y EINT1 por flanco
    LPC_SC->POLAR |= (1<<0);                  // EINT0 por flanco de subida
    LPC_SC->POLAR &= ~(1<<1);                 // EINT1 por flanco de bajada
    LPC_SC->EXTINT |= (3<<0);                 // Limpia interrupciones pendientes
    NVIC_SetPriority(EINT0_IRQn, 3);          // Prioridad 3 para EINT0
    NVIC_SetPriority(EINT1_IRQn, 2);          // Prioridad 2 para EINT1
    NVIC_EnableIRQ(EINT0_IRQn);               // Habilita EINT0
    NVIC_EnableIRQ(EINT1_IRQn);               // Habilita EINT1
}

// Configura el temporizador SysTick
void cfgSysTick(uint32_t ticks){
    SysTick->VAL = 0;         // Limpia el registro de valor actual
    SysTick->LOAD = ticks;    // Establece el valor de recarga
    SysTick->CTRL = 0x07;     // Habilita SysTick, su interrupción y usa el reloj del procesador
}

// Manejador de interrupción para EINT0 (cambia la forma de onda)
void EINT0_IRQHandler(void){
    setWave = (setWave + 1)%NUM_WAVE; // Selecciona la siguiente forma de onda (circular)
    wave_offset = 8*setWave;          // Calcula el offset para la forma de onda seleccionada
    LPC_GPIO2->FIOPIN0 = average();   // Muestra el promedio en P2.0-P2.7
    LPC_SC->EXTINT |= (1<<0);         // Limpia la interrupción pendiente
}

// Manejador de interrupción para EINT1 (cambia el periodo de la señal)
void EINT1_IRQHandler(void){
    nTicks++;                         // Incrementa el contador de ticks
    if(nTicks%2 == 0){
        cfgSysTick(load_160);         // Cambia a periodo de 160ms
    }else{
        cfgSysTick(load_80);          // Cambia a periodo de 80ms
    }
    LPC_SC->EXTINT |= (1<<1);         // Limpia la interrupción pendiente
}

// Manejador de interrupción SysTick (genera la forma de onda en P2.8)
void SysTick_Handler(){
    static int bit_pos=0;              // Posición del bit actual en la forma de onda
    if(bit_pos < SIZE_WAVE){
        if(*waveForm & (1<<(bit_pos+wave_offset))){ // Si el bit es 1
            LPC_GPIO2->FIOSET = (1<<8);             // Pone P2.8 en alto
        }else{
            LPC_GPIO2->FIOCLR = (1<<8);             // Pone P2.8 en bajo
        }
        bit_pos++;          // Avanza al siguiente bit
    }
    SysTick->CTRL &= SysTick->CTRL; //Limpia Flag SysTick (COUNTFLAG)

}

// Calcula el promedio de la forma de onda seleccionada
uint8_t average(){
    uint8_t sum = 0; // Acumula la suma de los bits
    for(uint8_t i=0; i<SIZE_WAVE; i++){
        sum += *waveForm & (1<<(i+wave_offset)); // Extrae cada bit y lo suma 
    }
    return sum/SIZE_WAVE; // Retorna el promedio
}