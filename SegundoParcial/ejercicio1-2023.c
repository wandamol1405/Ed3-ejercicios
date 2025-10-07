/*
Programar el microcontrolador LPC1769 en un codigo de lenguaje C para que,
utilizando un timer y un pin de capture de esta placa sea posible demodular
una señal PWM que ingresa por dicho pin (calcular el ciclo de trabajo y el periodo)
y sacar una tension continua proporcional al ciclo de trabajo a traves del DAC de rango
dinamico 0-2V con un rate de actualizacion de 0.5s del promedio de los ultimos valores
obtenidos en la captura
*/

#include <LPC17xx.h>
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"

#define MAX_DAC_VALUE_FOR_2V 621   // 2V * 1024 / 3.3V
#define TIMER_TICK_US 1000            // Resolución 1 milisegundo

// Buffer para almacenar ciclos de trabajo capturados en 0.5s
#define BUFFER_SIZE 500 // Capacidad máxima teórica de muestras en 0.5s

volatile float duty_cycles_buffer[BUFFER_SIZE];
volatile uint16_t buffer_index = 0;

// Prototipos de funciones
void cfgPCB(void);
void cfgTimer0(void);
void cfgTimer1(void);

int main(void) {
    cfgPCB();
    cfgTimer0();
    cfgTimer1();
    DAC_Init(LPC_DAC); // Habilita power y configura clock
    while(1) {
        // El trabajo principal se realiza en las ISRs.
    }
}

void cfgPCB(void) {
    // Configuraciones de PinSEL y PinMODE para Timer 0 CAP0.0 
    PINSEL_CFG_Type PinCfg_CAP;
    PinCfg_CAP.Portnum = 1; 
    PinCfg_CAP.Pinnum = 26;
    PinCfg_CAP.Funcnum = 3;  // CAP0.0 (Función 3)  
    PinCfg_CAP.Pinmode = 2;  // Sin pull-up/down 
    PinCfg_CAP.OpenDrain = 0;
    PINSEL_ConfigPin(&PinCfg_CAP); 
    
    // Configuración DAC P0.26 (AOUT) Func 10 
    PINSEL_CFG_Type PinCfg_DAC;
    PinCfg_DAC.Portnum = 0; 
    PinCfg_DAC.Pinnum = 26;
    PinCfg_DAC.Funcnum = 2;  // AOUT (Función 2) 
    PinCfg_DAC.Pinmode = 2;
    PinCfg_DAC.OpenDrain = 0;
    PINSEL_ConfigPin(&PinCfg_DAC); 
}

void cfgTimer0(void) {
    // Usamos Timer 0 para la captura

    // Configuración base de tiempo (1 ms tick)
    TIM_TIMERCFG_Type cfgTimer0;
    cfgTimer0.PrescaleOption = TIM_PRESCALE_USVAL; 
    cfgTimer0.PrescaleValue = TIMER_TICK_US;
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &cfgTimer0); 

    // Configuración de captura (CAP0.0, canal 0)
    TIM_CAPTURECFG_Type cfgCapture;
    cfgCapture.CaptureChannel = 0;
    cfgCapture.RisingEdge = ENABLE;   // Captura en flanco de subida 
    cfgCapture.FallingEdge = ENABLE;  // Captura en flanco de bajada 
    cfgCapture.IntOnCaption = ENABLE; // Genera interrupción en captura 
    TIM_ConfigCapture(LPC_TIM0, &cfgCapture); 

    TIM_Cmd(LPC_TIM0, ENABLE); // Habilita el contador 
    NVIC_EnableIRQ(TIMER0_IRQn); // Habilita interrupción en NVIC 
}

void cfgTimer1(void) {
    // Usamos Timer 1 para la actualización de 0.5s

    // Configuración base de tiempo (1 us tick)
    TIM_TIMERCFG_Type tim_config;
    tim_config.PrescaleOption = TIM_PRESCALE_USVAL;
    tim_config.PrescaleValue = TIMER_TICK_US; 
    TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &tim_config);

    // Configuración de Match (MR0) para 500,000 uS (0.5s)
    TIM_MATCHCFG_Type match_config;
    match_config.MatchChannel = 0; 
    match_config.IntOnMatch = ENABLE;     // Interrumpe al hacer match 
    match_config.ResetOnMatch = ENABLE;   // Reinicia el contador (periódico) 
    match_config.StopOnMatch = DISABLE;   // No se detiene 
    match_config.ExtMatchOutputType = TIM_EXTMATCH_NOTHING; 
    match_config.MatchValue = 499; // 500 ticks - 1 
    TIM_ConfigMatch(LPC_TIM1, &match_config);

    TIM_Cmd(LPC_TIM1, ENABLE);
    NVIC_EnableIRQ(TIMER1_IRQn); // Habilita interrupción en NVIC 
}

void TIMER0_IRQHandler(void) {
    if (TIM_GetIntStatus(LPC_TIM0, TIM_CR0_INT) == SET) {
        uint32_t t_cap_raw = TIM_GetCaptureValue(LPC_TIM0, 0);

        static uint32_t t_rise_prev = 0;
        static uint32_t t_rise_curr = 0;
        static uint32_t t_fall = 0;

        // Detectar tipo de flanco según el estado de CAP
        if ((LPC_GPIO1->FIOPIN >> 26) & 1) { 
            // Es un flanco de subida
            t_rise_prev = t_rise_curr;
            t_rise_curr = t_cap_raw;

            if (t_rise_prev != 0 && t_fall > t_rise_prev) {
                uint32_t T = t_rise_curr - t_rise_prev; // Período
                uint32_t t_H = t_fall - t_rise_prev;    // Tiempo en alto

                float duty_cycle = (float)t_H / (float)T;

                // Guardar valor en buffer
                if (buffer_index < BUFFER_SIZE) {
                    duty_cycles_buffer[buffer_index++] = duty_cycle;
                }
            }

        } else {
            // Es un flanco de bajada
            t_fall = t_cap_raw;
        }

        TIM_ClearIntPending(LPC_TIM0, TIM_CR0_INT);
    }
}



// ISR para la actualización del DAC cada 0.5s
void TIMER1_IRQHandler(void) {
    if (TIM_GetIntStatus(LPC_TIM1, TIM_MR0_INT) == SET) { 
        
        // 1. Calcular el promedio de Duty Cycle (D_avg)
        float sum_D = 0.0;
        uint16_t count = buffer_index;
        
        for (int i = 0; i < count; i++) {
            sum_D += duty_cycles_buffer[i];
        }

        float D_avg = (count > 0) ? sum_D / count : 0.0;
        
        // 2. Escalar D_avg (0 a 1) a valor digital DAC (0 a MAX_DAC_VALUE_FOR_2V=621)
        uint32_t dac_value_digital = (uint32_t)(D_avg * MAX_DAC_VALUE_FOR_2V);
        
        // 3. Escribir al DAC
        DAC_UpdateValue(LPC_DAC, dac_value_digital);

        // 4. Resetear el buffer para la próxima ventana de 0.5s
        buffer_index = 0;


        TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT); // Limpiar bandera de Match 0 [16, 19]
    }
}
