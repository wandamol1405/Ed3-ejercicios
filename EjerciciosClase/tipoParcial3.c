/*
 En la fabrica, hay un sistema de alarma utilizando una LPC1769 Rev. D trabajando
 a una frecuencia de CCLK a 100 MHz, conectado a un sensor de puerta que se activa cuando
 la puerta se abre. El sensor está conectado al pin P0.6, el cual genera una interrupción externa
 EINT1 cuando se detecta una apertura.

 Al detectar que la puerta se ha abierto, el sistema debe iniciar un temporizador utilizando el SysTick
 para contar un periodo de 30 segundos. Durante estos 30 segundos, el usuario deberá introducir un
 código de desactivación mediante un DIP switch de 4 entradas conectado a los pines P2.0 a P2.3.
 El código correcto es 0xA (1010). El usuario tiene dos intentos para introducir el código correcto.
 Si después de dos intentos el código ingresado es incorrecto, la alarma se activará, encendiendo
 un buzzer conectado al pin P1.11.

 Resumen de conexiones:
 P2.11 -> EINT1 -> SENSOR DE LA PUERTA -> ACTIVA UN TEMPORIZADOR DE 30s PARA INGRESAR EL CODIGO
 P2.0 AL P2.3 -> GPIO ENTRADAS -> DIP SWITCH PARA EL CODIGO
 P1.10 -> GPIO SALIDA -> BUZZER SI EL CODIGO ES INCORRECTO
*/

#include "LPC17xx.h"

#define VALID_CODE 0xA         // Código de validación correcto
#define LOAD_VALUE 0xFFFFFF    // Valor de desbordamiento del SysTick
#define COUNT_OVERFLOWS 179    // Cantidad de desbordamientos para 30 segundos

static int countST = 0;        // Cantidad de desbordamientos a contar para llegar a 30 segundos
static int validWindow = 0;    // Flag para saber si se encuentra en la ventana de validación
static int tryValidation = 0;  // Cantidad de intentos de validación

void cfgGPIO(void);
void cfgEINT(void);
void cfgSysTick(void);

int main(void){
    cfgGPIO();
    cfgEINT();
    cfgSysTick();
    return 0;
}

void cfgGPIO(){
    LPC_PINCON->PINSEL2 &= ~(3<<20);        // P1.10 como GPIO
    LPC_PINCON->PINSEL4 &= ~(0xFF<<0);      // P2.0 a P2.3 y P2.11 como GPIO
    LPC_GPIO1->FIODIR |= (1<<10);           // P1.10 como salida
    LPC_GPIO2->FIODIR &= ~(0xF<<0);         // P2.0 a P2.3 como entradas
    LPC_GPIO1->FIOCLR |= (1<<10);           // Apago el buzzer
}

void cfgEINT(){
    LPC_PINCON->PINSEL4 |= (1<<22);         // P2.11 como EINT1
    LPC_SC->EXTMODE |= (1<<1);              // Configuro la interrupción por flanco
    LPC_SC->EXTPOLAR |= (1<<1);             // Configuro la interrupción por flanco ascendente
    NVIC_SetPriority(EINT1_IRQn, 1);        // Setea la prioridad de la interrupción (media)
    NVIC_EnableIRQn(EINT1_IRQn);            // Habilita la interrupción externa
}

void cfgSysTick(){
    SysTick->LOAD = LOAD_VALUE;             // Recarga el valor de desbordamiento
    SysTick->VAL = 0;                       // Pone el valor actual en 0
    SysTick->CTRL = 0x07;                   // Habilita el SysTick, la interrupción y define la fuente de clock
}

void EINT1_IRQHandler(){
    if(!validWindow){
        validWindow = 1;                    // Activo la ventana de validación
        countST = COUNT_OVERFLOWS;          // Configuro el SysTick para contar 30 segundos
        tryValidation = 0;                  // Reinicio la cantidad de intentos de validación
    }
}

void SysTick_Handler(){
    if(countST > 0){                        // Si no se ha terminado el conteo
        countST--;                          // Decremento el contador
    }else{
        if(validWindow){                    // Si se terminó la ventana de validación
            validWindow = 0;                // Desactivo la ventana de validación
            uint32_t code = LPC_GPIO2->FIOPIN & 0xF;   // Leo el código ingresado
            if((code != VALID_CODE) && (tryValidation > 0)){ // Si el código es incorrecto y ya se intentó validar
                LPC_GPIO1->FIOSET |= (1<<10);           // Enciendo el buzzer
                tryValidation = 0;                      // Reinicio la cantidad de intentos de validación
            }else if(!tryValidation){                   // Si el código es incorrecto y es el primer intento
                tryValidation++;                        // Incremento la cantidad de intentos de validación
                countST = COUNT_OVERFLOWS;              // Configuro el SysTick para contar 30 segundos
                validWindow = 1;                        // Reactivo la ventana de validación
            }
        }
    }
}
