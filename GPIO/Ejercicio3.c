/*
 * Configurar el pin P0.4 como entrada digital con resistencia de pull down y
 * utilizarlo para decidir si el valor representado por los pines P0.0 al P0.3 van a
 * ser sumados o restados al valor guardado en la variable "acumulador".
 * El valor inicial de "acumulador" es 0.
 */

#include "LPC17xx.h"
#include <stdint.h>

void configPorts(void);
uint32_t getNum(void);

int main(void){
	// si P0.4 = 1, suma. de lo contratio, resta
	configPorts();
	uint32_t acumulador = 0;

	while(1){
		uint32_t num = getNum();
		if(LPC_GPIO0->FIOPIN & (1<<4)){ // P0.4 en 1 --> suma
			acumulador += num;
		}else{							// P0.4 en 0 --> resta
			acumulador -= num;
		}

	}
	return 0;

}

void configPorts(){
	// Configuracion de la entrada P0.4
    LPC_PINCON->PINSEL &= ~(0xFF); //P0.0 a P0.3 como GPIO
	LPC_PINCON->PINMODE0 &= ~(3 <<8); // limpio bits
	LPC_PINCON->PINMODE0 |= (3 << 8); // configura pull-down en el pin P0.4
	LPC_GPIO0->FIODIR &= ~(1<<4); // 1110 1111 solo el pin 4 como entrada
}

uint32_t getNum(){
	// obtiene el valor en los pines P0.0 a P0.3
	return (LPC_GPIO0->FIOPIN & 0x0F); // mascara 0000 1111, lee P0.0 a P0.3
}