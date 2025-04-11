/*! @mainpage Guia 1 Ejercicio 6 - Petracchi, F. M.
 *
 * @section genDesc Descripción General
 *
 * Este programa convierte un número decimal a su representación BCD y lo muestra en un
 * display LCD utilizando pines GPIO del ESP32. La función mostrarNumero() se encarga de
 * mostrar el número en el display, configurando los pines GPIO correspondientes a cada
 * dígito BCD.
 *
 * <a href="https://drive.google.com/file/d/1mUsnvuQx0knYz3CoCl0zhBYzHK4LwOqo/view?usp=drive_link">Ejemplo de operación brindado por la cátedra</a>
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
 * | 	GND	    	| 	GND		    |
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
/**
 * @def DEBUG_MODE
 * @brief Modo de depuración
 * @details 1 para habilitar "printf()"s de depuración, 0 para deshabilitar.
 */
#define DEBUG_MODE 0

/*==================[internal data definition]===============================*/

/**
 * @brief Estructura de configuración para pines GPIO
 * Esta estructura contiene los parámetros de configuración para un pin GPIO,
 * incluyendo su número de pin y dirección.
 * @typedef gpioConf_t
 * @struct gpioConf_t
 * @field pin Identificador del pin GPIO
 * @field dir Dirección del pin GPIO (entrada o salida)
 */
typedef struct {
	gpio_t  pin;	/**< Pin number */
	io_t  dir;		/**< Pin direction '0': IN, '1': OUT */
} gpioConf_t;

/**
 * @brief Mapea cada digito a su GPIO activador correspondiente
 */
const gpioConf_t diggit_mapping[3] = {
    {GPIO_9, GPIO_OUTPUT},   // Digito 1 (LSB)--> GPIO_9
    {GPIO_18, GPIO_OUTPUT},  // Digito 2 -------> GPIO_18
    {GPIO_19, GPIO_OUTPUT},  // Digito 3 (MSB)--> GPIO_19
};

/**
 * @brief Mapea los bits para representar un digito BCD sus correspondientes pines GPIO
 * @details Cada pin GPIO se asigna a un bit específico del número BCD.
 *          Los pines GPIO_20, GPIO_21, GPIO_22 y GPIO_23 se utilizan para
 *          controlar los segmentos del display LCD.
 */
const gpioConf_t lcd_mapping[4] = {
    {GPIO_20, GPIO_OUTPUT},  // b0 -> GPIO_20
    {GPIO_21, GPIO_OUTPUT},  // b1 -> GPIO_21
    {GPIO_22, GPIO_OUTPUT},  // b2 -> GPIO_22
    {GPIO_23, GPIO_OUTPUT}   // b3 -> GPIO_23
};

/*==================[internal functions declaration]=========================*/

/**
 * @brief Inicializa múltiples GPIOs según la configuración proporcionada
 * @param config Puntero a un arreglo de estructuras gpioConf_t que contiene la configuración de los GPIOs
 *               Cada elemento del array debe especificar el pin y su dirección
 * @return int8_t Retorna 0 si la inicialización fue exitosa
 * @note La función utiliza GPIOInit() para configurar cada GPIO individualmente
 */
int8_t inicializarGPIOs (const gpioConf_t *config)
{
    for (uint8_t i = 0; i < sizeof(config)/sizeof(config[0]); i++) {
        GPIOInit(config[i].pin, config[i].dir);
    }
    return 0; // Éxito
}

/**
 * @brief Convierte un número decimal a su representación BCD en un arreglo
 * @param num Número decimal de tipo uint32_t a convertir
 * @param digitos Cantidad de dígitos decimales a procesar
 * @param bcd_number Puntero al arreglo donde se almacenarán los dígitos BCD
 * @note Los dígitos se almacenan del menos significativo al más significativo.
 * @note El arreglo bcd_number debe tener suficiente espacio para almacenar todos los dígitos
 */
void convertToBcdArray (uint32_t num, uint8_t digitos, uint8_t * bcd_number)
{
	// Convierte cada dígito a 0 o 1 y lo almacena en el arreglo
	for (uint8_t i = 0; i < digitos; i++) {
        bcd_number[i] = num % 10;         // Obtiene el dígito menos significativo
        num /= 10;				// Elimina el dígito menos significativo para continuar con el siguiente
	}
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
    for (uint8_t i = 0; i < 4; i++) {
        // Establece el estado del GPIO según el bit correspondiente
        if (bcd & (1<<i)) { // Verifica si el bit i está en 1
            GPIOOn(config[i].pin);    // Bit en 1, enciende el GPIO
        } else {
            GPIOOff(config[i].pin);   // Bit en 0, apaga el GPIO
        }
    }
}

/**
 * @brief Muestra un número en un display BCD utilizando GPIOs
 * @details Esta función toma un número decimal, lo convierte a BCD (Binary-Coded Decimal)
 * y lo muestra en un display LCD. El display se controla mediante GPIOs configurados
 * para los dígitos BCD y para la selección de dígitos en el LCD.
 * @param numero Número decimal (0-999) de tipo uint32_t a mostrar
 * @param digitos Cantidad de dígitos a mostrar (1-3)
 * @param bcd_config Puntero a la configuración de GPIOs para selección de dígitos
 * @param lcd_config Puntero a la configuración de GPIOs para los segmentos del LCD
 * @return int8_t Retorna 0 si la operación fue exitosa, -1 si hubo un error
 * @note La función verifica que el número de dígitos sea válido antes de procesar
 * @note En modo DEBUG, la función imprime información detallada del proceso
 */
int8_t mostrarNumero(uint32_t numero, uint8_t digitos, const gpioConf_t *bcd_config, const gpioConf_t *lcd_config)
{
    // Verifica que el número de dígitos no exceda el tamaño del arreglo
    if ((digitos > (sizeof(diggit_mapping) / sizeof(diggit_mapping[0])) || (digitos < 1))) {
        return -1; // Error: número de dígitos inválido
    }

    #if DEBUG_MODE
        printf("Número a mostrar: %u\n", numero); // Imprime el número a mostrar
    #endif

    uint8_t bcd_digits[3];  // Arreglo para almacenar hasta 3 dígitos BCD

    // Convierte el número a un arreglo de dígitos BCD
    convertToBcdArray(numero, digitos, bcd_digits);

    #if DEBUG_MODE
        printf("Número BCD: ");
        for (uint8_t i = 0; i < digitos; i++) {
            printf("%d ", bcd_digits[i]); // Imprime el dígito BCD
        }
        printf("\n");
    #endif

    // Configura cada dígito en el display
    for (uint8_t i = 0; i < digitos; i++) {
        #if DEBUG_MODE
            printf("Configurando dígito %d: %d\n", i, bcd_digits[i]); // Imprime el dígito BCD
        #endif
                
        // Configura los GPIOs según el valor BCD del dígito
        BCDToGPIO(bcd_digits[i], lcd_config);

        #if DEBUG_MODE
            printf("Configurando GPIOs para el dígito %d\n", i); // Imprime el dígito BCD
        #endif

        // Envía un pulso encendiendo y apagando el GPIO correspondiente al dígito en el LCD
        GPIOOn(bcd_config[i].pin);
        GPIOOff(bcd_config[i].pin);
    }
    #if DEBUG_MODE
        printf("Número mostrado en el display\n");
    #endif

    return 0;
}

/*==================[external functions definition]==========================*/
void app_main(void){
	uint32_t num = 456; 		// Número a convertir a BCD
	uint8_t digitos = 3;		// Variable para almacenar el número BCD

    // Inicializa los GPIOs de BCD y LCD
    if (inicializarGPIOs(diggit_mapping) == 0 || inicializarGPIOs(lcd_mapping) == 0) {
        #if DEBUG_MODE
            printf("GPIOs inicializados correctamente\n");
        #endif
    } else {
        #if DEBUG_MODE
            printf("Error al inicializar GPIOs\n");
        #endif
    }

    // Muestra el número en el display
    if (mostrarNumero(num, digitos, &diggit_mapping, &lcd_mapping) == 0) {
        #if DEBUG_MODE
            printf("Número %u mostrado correctamente.\n", num);
        #endif
    } else {
        #if DEBUG_MODE
            printf("Error al mostrar el número\n");
        #endif
    }
}
/*==================[end of file]============================================*/