/*
* Escribir un programa en C que permita realizar un promedio móvil con los
* últimos 8 datos ingresados por el puerto 1. Considerar que cada dato es un
* entero signado y está formado por los 16 bits menos significativos de dicho
* puerto. El resultado, también de 16 bits, debe ser sacado por los pines P0.0 al
* P0.11 y P0.15 al P0.18. Recordar que en un promedio móvil primero se
* descarta el dato mas viejo de los 8 datos guardados, se ingresa un nuevo dato
* proveniente del puerto y se realiza la nueva operación de promedio con esos 8
* datos disponibles, así sucesivamente. Considerar el uso de un retardo antes de
* tomar una nueva muestra por el puerto.
 */
#include "LPC17xx.h"
#include <stdint.h>

void configPorts(void);
int32_t movingAverage(int16_t data[8]);
void showResult(int32_t result);
void delay(uint32_t delayTime);

int main(){
	configPorts();
	// Store data
	int16_t data[8];
	int32_t result;
	while(1){
		for(int i = 0; i<8; i++){
			data[i]=LPC_GPIO1->FIOPIN & 0xffff;
			delay(1000000);
		}
		result = movingAverage(data);
		showResult(result);
		delay(100000000);
	}
	return 0;
}


void configPorts(){
	// All GPIO
	LPC_PINCON->PINSEL0 &= ~(0x0FFF);
	LPC_PINCON->PINSEL1 &= ~(0x00FF);
	// Outputs without resistors, Inputs with pull-ups
	LPC_PINCON->PINMODE0 |= (0x80AAAAAA); // 1000 0000 1010 1010 1010 1010 1010 1010
	LPC_PINCON->PINMODE1 |= (0x2A);  // 00000000 00000000 00000000 00101010
	// no Open-Drain (default)
	// P1.0 to P1.15 INPUT, P0.0 to P0.11 and P0.15 to P0.18 OUTPUT
	LPC_GPIO0->FIODIR |= (0x0003C7FF);//00000000 00000011 11000111 11111111 o (0xF << 15)|(7 << 8)|(0xFF)
	LPC_GPIO1->FIODIR &= (0xFFFF); //00000000 00000000 11111111 11111111
	//no masks (default) -> could be to the pins of Port 0 that show nothing
}


int32_t movingAverage(int16_t data[8]){
	int32_t sum = 0;
	for(int i=0; i<8; i++){
		sum += data[i];
	}
	return sum/8;
}

void showResult(int32_t result){
	int16_t resultL = result & 0x0fff;
	int16_t resultH =(result >> 12) & 0xf;
	// clear output
	LPC_GPIO0->FIOCLR = 0x000F8FFF;
	LPC_GPIO0->FIOSET = resultL | (resultH << 15);
}