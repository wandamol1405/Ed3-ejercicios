/*
Programar el microcontrolador LPC1769 para que mediante su ADC digitalice dos señales
analogicas cuyos anchos de bandas son de 10kHz cada una. Los canales utilizados deben
ser el 2 y el 4 y los datos deben ser guardados en dos regiones de memorias distintas
que permitan contar con los 20 datos de cada canal. Suponer una frecuencia de core cclk 
de 100MHz. El codigo debe estar debidamente comentado.
*/
#include <LPC17xx.h>
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_gpdma.h"

#define FREQ_ADC 40000 // 40kHz
