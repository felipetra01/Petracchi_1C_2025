/*! @mainpage Guia1_E4y5 - Petracchi, F. M.
 *
 * @section genDesc Descripción General
 *
 * Este programa convierte un número decimal a su representación BCD y muestra su <b>primer dígito<\b> en un
 * display LCD utilizando pines GPIO del ESP32.
 * 
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	D1  	 	| 	GPIO_20		|
 * | 	D2  	 	| 	GPIO_21		|
 * | 	D3  	 	| 	GPIO_22		|
 * | 	D4  	 	| 	GPIO_23		|
 * | 	SEL1  	 	| 	GPIO_19		|
 * | 	SEL2  	 	| 	GPIO_18		|
 * | 	SEL3  	 	| 	GPIO_9		|
 * | 	Vcc(+5V)	| 	+5V		    |
 * | 	GND	    	| 	GND		    /
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 28/03/2025 | Document creation		                         |
 *
 * @author Felipe M. Petracchi (felipe.petracchi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct {
	gpio_t  pin;	/**< Pin number */
	io_t  dir;		/**< Pin direction '0': IN, '1': OUT */
} gpioConf_t;

const gpioConf_t bcd_mapping[4] = {	// Arreglo para almacenar la configuración de los pines GPIO
    {GPIO_20, 1},  // b0 -> GPIO_20
    {GPIO_21, 1},  // b1 -> GPIO_21
    {GPIO_22, 1},  // b2 -> GPIO_22
    {GPIO_23, 1}   // b3 -> GPIO_23
};
/*==================[internal functions declaration]=========================*/

/**
 * @brief Convierte un número decimal a su representación BCD.
 * @param data Número decimal a convertir.
 * @param digits Número de dígitos a convertir.
 * @param bcd_number Arreglo para almacenar el número BCD resultante.
 * @return 0 si la conversión fue exitosa, -1 si hubo un error.
 */
int8_t convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	int8_t i = 0;
	uint8_t binario = 0;

	// Verifica que el número de dígitos no exceda el tamaño del arreglo
	if (digits > 3) {
		return -1; // Error: número de dígitos inválido
	}
	// Convierte cada dígito a 0 o 1 y lo almacena en el arreglo
	for (i = 0; i < digits; i++) {
		binario = data % 10;	 // Obtiene el dígito menos significativo
		binario = (binario & 0x0F);
		bcd_number[i] = binario; // Almacena el digito binario en la posición del arreglo
		data /= 10;		 	 // Elimina el dígito menos significativo para continuar con el siguiente
	}
	return 0; // Éxito
}

/**
 * @brief Configura los GPIOs según el valor BCD de un dígito dado.
 * @param bcd Valor de 0-9 a configurar en los GPIOs
 * @param config Puntero a la configuración de los GPIOs
 * @note La función utiliza GPIOOn() y GPIOOff() para encender y apagar los pines según el valor BCD
 */
void BCDToGPIO(uint8_t bcd, const gpioConf_t *config)
{
    // Configura cada GPIO según el bit correspondiente en el BCD
    for (int i = 0; i < 4; i++) {
        // Inicializa el pin como salida
        GPIOInit(config[i].pin, config[i].dir);
        // Establece el estado del GPIO según el bit correspondiente
        if (bcd & (1 << i)) {
            GPIOOn(config[i].pin);    // Bit en 1, enciende el GPIO
        } else {
            GPIOOff(config[i].pin);   // Bit en 0, apaga el GPIO
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
	u_int32_t num = 9; 		// Número a convertir a BCD
	uint8_t digitos = 1;		// Variable para almacenar el número BCD
	uint8_t bcd_number[4];	// Arreglo para almacenar el número BCD

	// Convierte el número a BCD y almacena el resultado en bcd_number
	if (convertToBcdArray(num, digitos, bcd_number) == 0) {
		// Imprime el número BCD convertido
		printf("Número: %ld\n", num);
		printf("Número BCD: %02X\n", bcd_number[0]);
		//printf("Número BCD: %02X %02X %02X %02X\n", bcd_number[0], bcd_number[1], bcd_number[2], bcd_number[3]);
	} else {
		printf("Error: número de dígitos inválido\n");
	}
	
	BCDToGPIO(bcd_number[0], bcd_mapping); // Enciende los LEDs
}
/*==================[end of file]============================================*/