/*! @mainpage Guia 2 Actividad 1 - Petracchi, F. M.
 *
 * @section genDesc General Description
 *
 * Se diseñó el firmware de manera que cumple con las siguientes funcionalidades:
 * <ol>
 * 		<li>Mostrar distancia medida utilizando los leds de la siguiente manera: </li>
 * - Si la distancia es menor a 10 cm, apagar todos los LEDs.
 * - Si la distancia está entre 10 y 20 cm, encender el LED_1.
 * - Si la distancia está entre 20 y 30 cm, encender el LED_2 y LED_1.
 * - Si la distancia es mayor a 30 cm, encender el LED_3, LED_2 y LED_1.
 * 		<li>Mostrar el valor de distancia en cm utilizando el display LCD. </li>
 * 		<li>Usar TEC1 para activar y detener la medición. </li>
 * 		<li>Usar TEC2 para mantener el resultado (“HOLD”): sin pausar la medición. </li>
 * 		<li>Refresco de medición: 1 s (1000 ms). </li>
 * <\ol>
 * 
 * Notas.
 * - El cambiar el estado de los LEDs forma parte de , no de la medición
 * - Se utilizó un delay en cada tarea de 100ms
 * 
 * Se conectó a la EDU-ESP un sensor de ultrasonido HC-SR04 y una pantalla LCD y utilizando los drivers
 * provistos por la cátedra implementar la aplicación correspondiente. Se ha subido al repositorio el código.
 * Se incluyó en la documentación, realizada con doxygen, el diagrama de flujo.
 * 
 * Es un nuevo proyecto en el que modifique la actividad del punto 1 de manera de utilizar interrupciones para
 * el control de las teclas y el control de tiempos (Timers). 
 * 
 * Se creó un nuevo proyecto en el que modificó la actividad del punto 2 agregando ahora el puerto serie. Envíe los datos de las mediciones
 * para poder observarlos en un terminal en la PC, siguiendo el siguiente formato:
 *  - 3 dígitos ascii + 1 carácter espacio + dos caracteres para la unidad (cm) + cambio de línea “ \r\n”
 * Además debe ser posible controlar la EDU-ESP de la siguiente manera:
 *  - Con las teclas “O” y “H”, replicar la funcionalidad de las teclas 1 y 2 de la EDU-ESP
 *  + Usar “I” para cambiar la unidad de trabajo de “cm” a “pulgadas"
 *  + Usar “M” para implementar la visualización del máximo
 *  + Usar “F” para aumentar la velocidad de lectura, reduciendo de a 100 milisegundos el tiempo de lectura
 *  + Usar “S” para disminuir la velocidad de lectura, aumentando de a 100 milisegundos el tiempo de lectura
 * 
 * <a href="https://drive.google.com/file/d/1yIPn12GYl-s8fiDQC3_fr2C4CjTvfixg/view">Ejemplo de operación dado por la cátedra</a>
 *
 * @section diagrama Diagrama de flujo
 * 
 * <img alt="Guia2E1-Diagrama de flujo" src="../Guia2E1-Diagrama.png" width=95%/>
 * 
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  | 	ESP32-C6	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * | 	TRIGGER	 	| 	GPIO_2		|
 * | 	Vcc		 	| 	+5V			|
 * | 	GND		 	| 	GND			|
 * |  LED_1 (GREEN)	| 	GPIO_11		|
 * |  LED_2	(YELL.)	| 	GPIO_10		|
 * |  LED_3 (RED) 	| 	GPIO_5		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/04/2025 | Document creation		                         |
 *
 * @author Felipe M. Petracchi (felipe.petracchi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "neopixel_stripe.h"

/*==================[macros and definitions]=================================*/
/**
 * @def DEBUG_MODE
 * @brief Modo de depuración
 * @details 1 para habilitar "printf()"s de depuración, 0 para deshabilitar.
 */
#define DEBUG_MODE 1

#define SHOW_PRINTFS_ON 0

#define TOGGLE_PRINTFS_ON 0
/**
 * @def DELAY_MEDICION
 * @brief Delay que marca la tasa de refresco de la tarea de medición y muestra.
 * @details 1000 ms
 */
#define DELAY_MEASURE 1000		*1000
#define DELAY_MEASURE_STEP 200	*1000
#define DELAY_MEASURE_MAX 2000	*1000
#define DELAY_MEASURE_MIN 100   *1000
/**
 * @def LED_1
 * @brief Variable que routea la tecla numerada como "1" con su GPIO correspondiente.
 */
#define TEC1 SWITCH_1
/**
 * @def LED_2
 * @brief Variable que routea la tecla numerada como "2" con su GPIO correspondiente.
 */
#define TEC2 SWITCH_2

typedef enum distance_units { CM = 0, INCHES = 1};

/*==================[internal data definition]===============================*/
/**
 * @brief Variable que indica si se debe medir o no.
 * @details Se utiliza para habilitar o deshabilitar la medición.
 */
bool MEDIR = true;
/**
 * @brief Variable que indica si se debe mantener mostrando un resultado o no.
 * @details Se utiliza para habilitar o deshabilitar la modificación de los LEDs y valores mostrados en el LCD.
 */
bool HOLD = false;

bool SHOW_MAX = false;

u_short UNIT = 0;
/**
 * @brief identificador de tipo TaskHandle_t de la tarea "medirMostrar".
 * @details Se utiliza para la creación de la tarea y para su control.
 */
TaskHandle_t measureAndShow_task_handle = NULL;

void measureAndShow(void *param);

timer_config_t timer_measure_config = {
	.timer = TIMER_A,
	.period = DELAY_MEASURE,
	.func_p = measureAndShow,
	.param_p = NULL,
};

void initComponentes(void);
void encenderLedsSegunDistancia(uint16_t distancia);
void measureAndShow(void *param);
/**
 * @brief Tarea que mide la distancia, muestra el resultado en el LCD y enciende los LEDs.
*/
static void measureAndShowTask(void *pvParameter);
void UartSendDistance(uint32_t value);
void uartListen(void *pvParameter);
void if_TEC1_do(void);
void if_TEC2_do(void);

/**
 * @brief Estructura de configuración del UART.
 * @details Se define de forma global para ser utlizado en las funciones internas además de en el main
 */
serial_config_t uart_config = {
	.port = UART_PC,
	.baud_rate = 19200,
	.func_p = uartListen,
	.param_p = NULL,
};

/**
 * @brief Variable que almacena la distancia medida.
 */
uint32_t distancia = 0;
/**
 * @brief Variable que almacena la distancia a enviar.
 */
uint32_t distance_to_send = 0;

/*==================[internal functions declaration]=========================*/

/** @brief Inicializa los componentes del sistema. */
void initComponentes(void) {
	LedsInit();
	SwitchesInit();
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();

	TimerInit(&timer_measure_config);
	UartInit(&uart_config);

	static neopixel_color_t color;
	NeoPixelInit(BUILT_IN_RGB_LED_PIN, BUILT_IN_RGB_LED_LENGTH, &color);
    NeoPixelAllOff();
}

void encenderLedsSegunDistancia(uint16_t distancia) {
	// encender o apagar LEDs segun distancia.
	if (distancia < 10) {
		LedOff(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
		NeoPixelAllOff();
		#if DEBUG_MODE && SHOW_PRINTFS_ON
			printf("Estado LEDs: 0 0 0\n"); // Print LEDs status
		#endif
	} else if (distancia < 20) {
		LedOn(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
		NeoPixelAllColor(NEOPIXEL_COLOR_GREEN);
		#if DEBUG_MODE	&& SHOW_PRINTFS_ON
			printf("Estado LEDs: 1 0 0\n"); // Print LEDs status
		#endif
	} else if (distancia < 30) {
		LedOn(LED_1);
		LedOn(LED_2);
		LedOff(LED_3);
		NeoPixelAllColor(NEOPIXEL_COLOR_YELLOW);
		#if DEBUG_MODE && SHOW_PRINTFS_ON
			printf("Estado LEDs: 1 1 0\n"); // Print LEDs status
		#endif
	} else {
		LedOn(LED_1);
		LedOn(LED_2);
		LedOn(LED_3);
		NeoPixelAllColor(NEOPIXEL_COLOR_RED);
		#if DEBUG_MODE && SHOW_PRINTFS_ON
			printf("Estado LEDs: 1 1 1\n"); // Print LEDs status
		#endif
	}
}

void measureAndShow(void *param) {
	vTaskNotifyGiveFromISR(measureAndShow_task_handle, pdFALSE);
}

static void measureAndShowTask(void *pvParameter){
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (MEDIR == true) {
			distancia = HcSr04ReadDistanceInCentimeters();
			// distancia = 1 + rand() % 39;	// Simulate a distance between 1 and 40 cm
			// distancia = 16;					// Simulate a specific distance
			#if DEBUG_MODE && SHOW_PRINTFS_ON
				printf("Distancia: %lu cm\n", distancia); // Print the measured distance
			#endif

			if (HOLD == false) {
				if ((SHOW_MAX && (distancia > distance_to_send)) || !SHOW_MAX) {
					distance_to_send = distancia;
				}
				encenderLedsSegunDistancia(distance_to_send);
				LcdItsE0803Write(distance_to_send);
				#if DEBUG_MODE	&& SHOW_PRINTFS_ON
					printf("LCD: %lu cm\n", distancia);
				#endif
			}
			UartSendDistance(distance_to_send); // Send the distance via UART
		}
	}
}

void UartSendDistance(uint32_t value) {
	if (UNIT == INCHES) {
		char str[10];
		float inches = value / 2.54f;
		sprintf(str, "%.2f", inches);
		UartSendString(uart_config.port, str);
		UartSendString(uart_config.port, " in\r\n");
	} else {		
		UartSendString(uart_config.port, (char *)UartItoa(value, 10));
		UartSendString(uart_config.port, " cm\r\n");
	}
}

void if_TEC1_do(){
	MEDIR = !MEDIR;
	if (MEDIR == false)	{
		LcdItsE0803Off();				// Apagar el LCD
		encenderLedsSegunDistancia(1);	// Apagar todos los LEDs
	}
	#if DEBUG_MODE	&& TOGGLE_PRINTFS_ON
		printf("\nToggled MEDIR from: %d --> %d\n", !MEDIR, MEDIR);
	#endif
}

void if_TEC2_do(){
	if (MEDIR) {
		HOLD = !HOLD;
		#if DEBUG_MODE	&& TOGGLE_PRINTFS_ON
			printf("\nToggled HOLD from: %d --> %d\n", !MEDIR, MEDIR);
		#endif
	}
}

void uartListen(void *pvParameter) {
    uint8_t data;
	UartReadByte(UART_PC, &data);
	switch (data) {
		case 'O':
			if_TEC1_do();
			break;
		case 'H':
			if_TEC2_do();
			break;
		case 'I':
			if (UNIT == CM)
			{ UNIT = INCHES; }
			else
			{ UNIT = CM; }
			break;
		case 'M':
			SHOW_MAX = !SHOW_MAX;
			#if DEBUG_MODE	&& TOGGLE_PRINTFS_ON
				printf("\nToggled MAX from: %d --> %d\n", !SHOW_MAX, SHOW_MAX);
			#endif
			break;
		case 'F':
			if (timer_measure_config.period > DELAY_MEASURE_MIN) {
				timer_measure_config.period -= DELAY_MEASURE_STEP;
				TimerUpdatePeriod(timer_measure_config.timer, timer_measure_config.period);
			}
			#if DEBUG_MODE	&& TOGGLE_PRINTFS_ON
				printf("\nToggled DELAY from: %lu ms --> %lu ms\n", (timer_measure_config.period + DELAY_MEASURE_STEP)/1000, timer_measure_config.period /1000);
			#endif
			break;
		case 'S':
			if (timer_measure_config.period < DELAY_MEASURE_MAX) {
				timer_measure_config.period += DELAY_MEASURE_STEP;
				TimerUpdatePeriod(timer_measure_config.timer, timer_measure_config.period);
			}
			#if DEBUG_MODE	&& TOGGLE_PRINTFS_ON
				printf("\nToggled DELAY from: %lu ms --> %lu ms\n", (timer_measure_config.period - DELAY_MEASURE_STEP)/1000, timer_measure_config.period /1000);
			#endif
			break;
		default:
			break;
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	initComponentes();

	uint32_t stack_size = 1024;
	#if DEBUG_MODE
		stack_size = 16384; // Increase stack size por los printf()s de depuración
	#endif
	
	SwitchActivInt(SWITCH_1, if_TEC1_do, NULL);
	SwitchActivInt(SWITCH_2, if_TEC2_do, NULL);
	
	xTaskCreate(&measureAndShowTask, "MEASUREANDSHOW", stack_size, NULL, 5, &measureAndShow_task_handle);

	TimerStart(timer_measure_config.timer);
}
/*==================[end of file]============================================*/