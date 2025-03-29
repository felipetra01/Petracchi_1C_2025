/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
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

static const gpioConf_t bcd_mapping[3] = {		// Mapea cada digito a su GPIO activador correspondiente
    {GPIO_9, GPIO_OUTPUT},   // Digito 1 (LSB)--> GPIO_9
    {GPIO_18, GPIO_OUTPUT},  // Digito 2 -------> GPIO_18
    {GPIO_19, GPIO_OUTPUT},  // Digito 3 (MSB)--> GPIO_19
};

static const gpioConf_t lcd_mapping[4] = {	// Mapea los bits de cada digito a los pines GPIO
    {GPIO_20, GPIO_OUTPUT},  // b0 -> GPIO_20
    {GPIO_21, GPIO_OUTPUT},  // b1 -> GPIO_21
    {GPIO_22, GPIO_OUTPUT},  // b2 -> GPIO_22
    {GPIO_23, GPIO_OUTPUT}   // b3 -> GPIO_23
};

/*==================[internal functions declaration]=========================*/

int8_t  convertToBcdArray (uint32_t num, uint8_t digitos, uint8_t * bcd_number)
{
    //uint8_t aux = 0;
	// Verifica que el número de dígitos no exceda el tamaño del arreglo
	if (digitos > 3) {
		return -1; // Error: número de dígitos inválido
	}
	// Convierte cada dígito a 0 o 1 y lo almacena en el arreglo
	for (uint8_t i = 0; i < digitos; i++) {
        bcd_number[i] = num % 10;         // Obtiene el dígito menos significativo
        num /= 10;				// Elimina el dígito menos significativo para continuar con el siguiente
	}
	return 0; // Éxito
}

void bcd_to_gpio(uint8_t bcd, gpioConf_t *config)
{
    // Configura cada GPIO según el bit correspondiente en el BCD
    for (uint8_t i = 0; i < 4; i++) {
        // Establece el estado del GPIO según el bit correspondiente
        if (bcd & (1<<i)) { // Verifica si el bit i está en 1
            GPIOOn(config[i].pin);    // Bit en 1, enciende el GPIO
        } else {
            GPIOOff(config[i].pin);   // Bit en 0, apaga el GPIO
        }
    }
}

int8_t mostrar_numero(uint32_t numero, uint8_t digitos, gpioConf_t *bcd_config, gpioConf_t *lcd_config)
{
    uint8_t bcd_digits[3];  // Arreglo para almacenar hasta 3 dígitos BCD

    // Convierte el número a un arreglo de dígitos BCD
    convertToBcdArray(numero, digitos, &bcd_digits);

    // Configura cada dígito en el display
    for (uint8_t i = 0; i < digitos; i++) {
        //GPIOOff(lcd_config[i].pin);  // Apaga el dígito inicialmente
        
        // Configura los GPIOs según el valor BCD del dígito
        bcd_to_gpio(bcd_digits[i], &lcd_config);
        
        // Enciende el dígito correspondiente en el LCD
        GPIOOn(bcd_config[i].pin);
        GPIOOff(bcd_config[i].pin);
    }
    
    return 0;
}

/*==================[external functions definition]==========================*/
void app_main(void){
	uint32_t num = 456; 		// Número a convertir a BCD
	uint8_t digitos = 3;		// Variable para almacenar el número BCD

    for(uint8_t i=0; i<3; i++){
        GPIOInit(bcd_mapping[i].pin, bcd_mapping[i].dir); // Inicializa los pines GPIO
    }

    for(uint8_t i=0; i<4; i++){
        GPIOInit(lcd_mapping[i].pin, lcd_mapping[i].dir); // Inicializa los pines GPIO
    }

	mostrar_numero(num, digitos, &bcd_mapping, &lcd_mapping);
    /*
    printf("Número: %ld\n", num);
    } else {
    printf("Error: error en la conversión del número\n");
    }
    */
}
/*==================[end of file]============================================*/