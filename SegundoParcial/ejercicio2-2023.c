/*
Programar el microcontrolador LPC1769 en un código de lenguaje C para que mediante
su ADC digitalice una señal analógica cuyo ancho de banda es de 16 Khz. 
La señal analógica tiene una amplitud de pico máxima positiva de 3.3 voltios.

Los datos deben ser guardados utilizando el Hardware GDMA en la primera mitad de la
memoria SRAM ubicada en el bloque AHB SRAM - bank 0, de manera tal que permita almacenar
todos los datos posibles que esta memoria nos permita. Los datos deben ser almacenados
como un buffer circular conservando siempre las últimas muestras.

Por otro lado se tiene una forma de onda como se muestra en la imagen a continuación.
Esta señal debe ser generada por una función y debe ser reproducida por el DAC desde
la segunda mitad de AHB SRAM - bank 0 memoria utilizando DMA de tal forma que se logre
un periodo de 614us logrando la máxima resolución y máximo rango de tensión.

Durante operación normal se debe generar por el DAC la forma de onda mencionada como wave_form.
Se debe indicar cuál es el mínimo incremento de tensión de salida de esa forma de onda.

Cuando interrumpe una extint conectada a un pin, el ADC configurado debe completar el ciclo de
conversión que estaba realizando, y ser detenido, a continuación se comienzan a sacar las muestras
del ADC por el DAC utilizando DMA y desde las posiciones de memoria originales.

Cuando interrumpe nuevamente en el mismo pin, se vuelve a repetir la señal del DAC generada por
la forma de onda de wave_form previamente almacenada y se arranca de nuevo la conversión de datos del ADC.
Se alterna así entre los dos estados del sistema con cada interrupción externa.

Suponer una frecuencia de core clk de 80 Mhz. El código debe estar debidamente comentado.
*/

