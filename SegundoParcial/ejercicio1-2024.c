/*
Por un pin del ADC del microcontrolador LPC1769 Rev. D. ingresa una tensión de rango dinámico −2V a 2V 
con una frecuencia máxima de 20kHz, obtenidos de un Sensor de presión diferencial que mide las variaciones 
de voltaje en función de la diferencia de presión a lo largo de una tubería de GNL. Se pide almacenar la señal, 
cumpliendo criterio de Nyquist, en la posición de memoria 0x2000E000 y con un tamaño 1kB, una vez ocupado todo 
el espacio, se vuelve a almacenar desde el comienzo.

En función del promedio de todas las muestras obtenidas cada 100ms de la señal capturada, se debe tomar una 
decisión sobre dos pines de salida GPIO que generan señales cuadradas de 3.3V en fase (Señales S1 y S2), 
usadas para ajustar dos válvulas de control de flujo que regulan la presión del gas en diferentes secciones 
de la planta y con una frecuencia de 10kHz:

- Si el promedio se encuentra en el rango −2V<Prom<0V, la señal de control de la válvula S1 se adelanta respecto a la de S2 
  en un ángulo proporcional al valor promedio (Rango de 0 a 180 grados, o π radianes).

- Si el promedio se encuentra en el rango 0V≤Prom<2V, la señal de control de la válvula S2 se adelanta respecto a la de S1
  en un ángulo proporcional al valor promedio (Rango de 0 a 180 grados, o π radianes).

  A CHEQUER :p
*/

#include <LPC17xx.h>
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_dma.h"
#include <math.h>
#include <stdint.h>

#define FREQ_ADC 40000 // 40kHz
#define BUFFER_SIZE (1024 / sizeof(uint32_t)) // 1kB
#define MATCH_VALUE_ADC 391 - 1 // 25us -> 40kHz
#define MATCH_VALUE_S 500 - 1 // 100us -> 10kHz
#define MAX_VALUE 4095
#define V_OFFSET 1.65

uint32_t *buffer = (uint32_t *) 0x2000E000;

void cfgPCB(){
    PINSEL_CFG_Type pin;

    // Configuro el canal 0 del ADC
    pin.Portnum = 0;
    pin.Pinnum = 23;
    pin.Funcnum = 1;
    pin.Pinmode = PINSEL_PINMODE_TRISTATE;
    pin.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&pin);

    // Configuro los pines de salida S1 y S2
    pin.Portnum = 0;
    pin.Pinnum = 0; // S1
    pin.Funcnum = 0;
    GPIO_SetDir(pin.Portnum, pin.Pinnum, 1);
    PINSEL_ConfigPin(&pin);

    pin.Pinnum = 1; // S2
    GPIO_SetDir(pin.Portnum, pin.Pinnum, 1);
    PINSEL_ConfigPin(&pin);

    // Configuro el pin MAT0.1 para la señal de 20kHz
    pin.Portnum = 1;
    pin.Pinnum = 29;
    pin.Funcnum = 3;

    PINSEL_ConfigPin(&pin);
}

void cfgADC(){
    ADC_Init(LPC_ADC, FREQ_ADC);
    ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_RISING);
    ADC_ChannelCmd(LPC_ADC, 0, ENABLE);
    ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
    NVIC_DisableIRQ(ADC_IRQn);
    ADC_BurstCmd(LPC_ADC, DISABLE);
    ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT01);
}

void cfgTimer(){
    TIM_TIMERCFG_Type timer;
    timer.PrescaleOption = TIM_PRESCALE_USVAL;
    timer.PrescaleValue = 1;

    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timer);

    TIM_MATCHCFG_Type matcher;
    // Configuracion del MR1 para iniciar conversiones ADC
    matcher.MatchChannel = 1;
    matcher.IntOnMatch = DISABLE;
    matcher.ResetOnMatch = ENABLE;
    matcher.StopOnMatch = DISABLE;
    matcher.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
    matcher.MatchValue = MATCH_VALUE_ADC;

    TIM_ConfigMatch(LPC_TIM0, &matcher);

    // Configuracion del MR0 para la señal de 10kHz
    matcher.MatchChannel = 0;
    matcher.IntOnMatch = ENABLE;
    matcher.ResetOnMatch = DISABLE;
    matcher.StopOnMatch = DISABLE;
    matcher.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    matcher.MatchValue = MATCH_VALUE_S;

    TIM_ConfigMatch(LPC_TIM0, &matcher);

    matcher.MatchChannel = 2;

    TIM_ConfigMatch(LPC_TIM0, &matcher);
    TIM_Cmd(LPC_TIM0, ENABLE);
    NVIC_EnableIRQ(TIMER0_IRQn);
}

void cfgDMA(){
    GPDMA_Init();
    GPDMA_LLI_Type lli;

    lli.SrcAddr = (uint32_t)&LPC_ADC->ADDR0;
    lli.DstAddr = (uint32_t)buffer;
    lli.NextLLI = (uint32_t)&lli;
    lli.Control = (BUFFER_SIZE<<0)|(2<<18)|(2<<21)|(1<<27)|(1<<31);

    GPDMA_Channel_CFG_Type channel;
    channel.ChannelNum = 0;
    channel.SrcMemAddr = 0;
    channel.DstMemAddr = (uint32_t)buffer;
    channel.TransferSize = BUFFER_SIZE;
    channel.TransferType = GPDMA_TRANSFERTYPE_P2M;
    channel.SrcConn = GPDMA_CONN_ADC;
    channel.DstConn = 0;
    channel.DMALLI = (uint32_t)&lli;
    GPDMA_Setup(&channel);
    GPDMA_ChannelCmd(0, ENABLE);
    NVIC_EnableIRQ(DMA_IRQn);
}

int main(){
    cfgPCB();
    cfgADC();
    cfgTimer();
    cfgDMA();
    while(1) {}
}

void DMA_IRQHandler(void) {
    if (GPDMA_IntGetStatus(GPDMA_STAT_INT, DMA_CHANNEL)) {
        uint64_t sum = 0;
        for (uint32_t i = 0; i < BUFFER_SIZE; ++i) {
            uint32_t raw = buffer[i];
            uint32_t code = (raw >> 4) & 0x0FFF;
            sum += code;
        }

        float avg_code_f = (float)sum / (float)BUFFER_SIZE; // 0..4095 (float)

        // --- Mapeo directo a -2..+2 V cuando el ADC está referenciado así ---
        float vin = (avg_code_f / 4095.0f) * 4.0f - 2.0f; // -2 .. +2 V

        // Período en microsegundos (T = 100 us para 10 kHz)
        const float PERIOD_US = 100.0f;
        const float HALF_US = PERIOD_US / 2.0f;

        // dt en microsegundos: dt = PERIOD_US * |code/4095 - 0.5|
        float dt_f = PERIOD_US * fabsf((avg_code_f / 4095.0f) - 0.5f);
        if (dt_f > HALF_US) dt_f = HALF_US; // seguridad numérica
        uint32_t dt_ticks = (uint32_t)roundf(dt_f); // ticks = microsegundos (prescaler = 1 us)

        // Asignación de MR2 según signo de vin
        if (vin < 0.0f) {
            // S1 adelanta a S2 => S2 debe togglear AFTER S1 -> MR2 = HALF + dt
            LPC_TIM0->MR2 = (uint32_t)(HALF_US) + dt_ticks;
        } else {
            // S2 adelanta a S1 => S2 togglea BEFORE S1 -> MR2 = HALF - dt
            // evitar underflow:
            uint32_t base = (uint32_t)(HALF_US);
            if (dt_ticks > base) dt_ticks = base;
            LPC_TIM0->MR2 = base - dt_ticks;
        }

        GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, DMA_CHANNEL);
    }
    NVIC_ClearPendingIRQ(DMA_IRQn);
}



void TIMER0_IRQHandler(){
    if(TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)==SET){
        LPC_GPIO0->FIOPIN ^= (1<<0); // Toggle S1
        LPC_TIM0->MR0 = MATCH_VALUE_S; // Reseteo MR0
        TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
    }else if(TIM_GetIntStatus(LPC_TIM0, TIM_MR2_INT)==SET){
        LPC_GPIO0->FIOPIN ^= (1<<1); // Toggle S2
        LPC_TIM0->MR2 = MATCH_VALUE_S; // Reseteo MR2
        TIM_ClearIntPending(LPC_TIM0, TIM_MR2_INT);
    }
    NVIC_ClearPendingIRQ(TIMER0_IRQn);
}