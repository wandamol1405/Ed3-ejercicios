/*
 Un estacionamiento automatico utiliza una barrera que se abre y cierra en funcion de la validacion de un ticket
 de acceso utilizando una LPC1769 Rev B trabajando a una frecuencia de CCLK a 70MHz.
 Cuando el sistema detecta que un automovil se ha posicionado frente a la bararera, se debe activar un sensor conectado al pin P2.10
 mediante una interrupcion externa EINT0. Una vez valido el ticket, se debe activar un motor que abre la barrera usando el pin P0.15.
 El motor debe estar activado por X segundos y luego apagarse, utilizando el temporizador SysTick para contar el tiempo.
 Si el ticket es invalido, se encendera un LED rojo conectado al pin P1.8.
 Para gestionar el teimpo de apertura de la barrera, existe un switch conectado al pin P2.1 que dispone de una ventana de configuracion
 de 3 segundos gestionada por el temporizador SysTick.
 Durante dicha ventana, se debe contar cuantas veces se presiona el switch y en funcion de dicha cantidad, establecer el tiempo de la barrera>
 de la siguiente manera:
    Cantidad de presiones del switch - Tiempo de apertura de la barrera - Cantidad de cuentas
                0                   -       5 segundos                   -    20 veces
                1                   -       10 segundos                  -    42 veces
                2                   -       20 segundos                  -    83 veces
                3                   -       40 segundos                  -    167 veces
                4                   -       5 segundos                   -    20 veces

Para contar 3 segundos se debe recargar 12 veces

P2.4 -> GPIO ENTRADA -> LECTURA DEL TICKET (valido 1, invalido 0)
P2.10 -> EINT0 -> SENSOR DEL AUTOMOVIL PRESENTE
P0.15 -> GPIO SALIDA -> MOTOR DE LA BARRERA -> si el ticket es valido
P1.8 -> GPIO SALIDA -> LED ROJO -> si el ticket es invalido
P2.1 -> GPIO INTERRUPCION -> CONFIGURACION DE TIEMPO DE APERTURA DE LA BARRERA
*/

#include "LPC17xx.h"

static uint32_t loadValue = 0xFFFFFF;  	// siempre se cuenta hasta desbordar lo maximo
static int countST = 20; 					// por default, se esperan 5 segundos
static int windowFlag = 0;		// sin intento de cambiar tiempo por defecto
static int countCfg = 0;		// 5 segundos por defecto
static int gateFlag = 0;        // cerrada por defecto

void cfgGPIO(void);
void cfgGPIOInt(void);
void cfgEINT(void);
void cfgSystTick(void);

int main(void){
	cfgGPIO();
	cfgGPIOInt();
	cfgEINT();
	cfgSysTick();
	return 0;
}

void cfgGPIO(){
	LPC_PINCON->PINSEL0 &= ~(3<<30);   			// P0.15 como GPIO
	LPC_PINCON->PINSEL2 &= ~(3<<16);			// P1.8 como GPIO
	LPC_PINCON->PINSEL4 &= ~(3<<2)|(3<<8);		// P2.1  y P2.4 como GPIO
	LPC_GPIO0->FIODIR |= (1<<15);				// P0.15 como salida
	LPC_GPIO1->FIODIR |= (1<<8);				// P1.8 como salida
	LPC_GPIO2->FIODIR &= ~((1<<1)|(1<<4));		// P2.1 y P2.4 como entradas
}

void cfgGPIOInt(){
	LPC_GPIOINT->IO2IntEnF |= (2<<0);    		// Configuro la interrupcion por flanco descendente
	LPC_GPIOINT->IO2IntClr |= (2<<0);			// Limpia la flag de interrupcion
	NVIC_SetPriority(EINT3_IRQn, 2);			// Setea la prioridad de la interrupcion -> mas baja
	NVIC_EnableIRQn(EINT3_IRQn);				// Habilita la interrupcion por GPIO
}

void cfgEINT(){
	LPC_SC->EXTMODE &= ~(1<<0);					// Configuro la interrupcion por flanco
	LPC_SC->EXTPOLAR |= (1<<0);					// Configuro la interrupcion por flanco ascendente
	LPC_SC->EXTINT |= (1<<0);					// Limpio la flag de interrupcion externa
	NVIC_SetPriority(EINT0_IRQn, 1);			// Setea la prioridad de la interrupcion -> media
	NVIC_EnableIRQn(EINT0_IRQn);				// Habilita la interrupcion externa
}

void cfgSysTick(){
	SysTick->LOAD = loadValue;					// Recarga el valor de desbordamiento
	SysTick->VAL = 0;							// Pone el valor actual en 0
	SysTick->CTRL = 0x07;						// Habilita el SysTick, la interrupcion y define la fuente de clock
}
// Auto presente -> revisar ticket
void EINT0_IRQHandler(){
	if(LPC_GPIO2->FIOPIN && (1<<4)){			// Si el ticket es valido
		LPC_GPIO1->FIOCLR |= (1<<8);			// Apago el led
		LPC_GPIO0->FIOSET |= (1<<15);			// Enciendo el motor
		gateFlag=1;								// SysTick contando el tiempo de motor encendido
		cfgSysTick();
		if(gateFlag&0){							// Si se termino el tiempo de conteo
			LPC_GPIO0->FIOCLR |= (1<<15);		// Cierra la barrera
		}
	}else{										// Si el ticket es invalido
		LPC_GPIO1->FIOSET |= (1<<8);			// Enciendo el LED
	}
	LPC_SC->EXTINT |= (1<<0);					// Limpio la flag de interrupcion externa
}

void EINT3_IRQHandler(){   // Parte importante a ver, ya que asume recursividad
	if(countCfg == 0){ // Si es la primera vez que se presiona el switch
		windowFlag++; // Activo la ventana de configuracion
		countCfg++;  // Incremento la cantidad de veces que se presiono el switch
		countST = 12; // Configuro el SysTick para contar 3 segundos
	}else{
		countCfg++;  // Incremento la cantidad de veces que se presiono el switch
	}

	LPC_GPIOINT->IO2IntClr |= (2<<0);
}

void SysTick_Handler(){
	if(countST > 0){
		countST--;
	}else{
		if(windowFlag){ // Si estoy en la ventana de configuracion
			windowFlag = 0; // Cierro la ventana de configuracion
			switch(countCfg){ // Veo cuantas veces se presiono el switch
				case 0:
					countST = 20; // 5 segundos
					break;
				case 1:
					countST = 42; // 10 segundos
					break;
				case 2:
					countST = 83; // 20 segundos
					break;
				case 3:
					countST = 167; // 40 segundos
					break;
				default:
					countST = 20; // 5 segundos
					break;
			}
			countCfg = 0; // Reseteo la cantidad de veces que se presiono el switch
		} else if(gateFlag){ // Si estoy en el tiempo de apertura de la barrera
			gateFlag = 0; // Cierro la barrera
			LPC_GPIO0->FIOCLR |= (1<<15); // Apago el motor
		}
	}
	SysTick->CTRL &= SysTick->CTRL;
}
