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
 * | 	LED_1	 	| 	GPIO_11		|
 * | 	LED_2	 	| 	GPIO_10		|
 * | 	LED_3	 	| 	GPIO_5		|
 *
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
#include "neopixel_stripe.h"

/*==================[macros and definitions]=================================*/
/**
 * @def DELAY_TASK
 * @brief Delay por defecto que marca la tasa de refresco de una tareas.
 * @details 100 ms
 */
#define DELAY_TASK 100
/**
 * @def DELAY_MEDICION
 * @brief Delay que marca la tasa de refresco de la tarea de medición y muestra.
 * @details 1000 ms
 */
#define DELAY_MEDICION 500
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
/**
 * @def DEBUG_MODE
 * @brief Modo de depuración
 * @details 1 para habilitar "printf()"s de depuración, 0 para deshabilitar.
 */
#define DEBUG_MODE 0

/*==================[internal data definition]===============================*/
/**
 * @brief identificador de tipo TaskHandle_t de la tarea "medirMostrar".
 * @details Se utiliza para la creación de la tarea y para su control. */
TaskHandle_t medirMostrarTask_handle = NULL;
/**
 * @brief identificador de tipo TaskHandle_t de la tarea "teclas".
 * @details Se utiliza para la creación de la tarea y para su control. */
TaskHandle_t teclasTask_handle = NULL;

/**
 * @brief Variable que indica si se debe medir o no.
 * @details Se utiliza para habilitar o deshabilitar la medición. */
bool MEDIR = true;
/**
 * @brief Variable que indica si se debe mantener mostrando un resultado o no.
 * @details Se utiliza para habilitar o deshabilitar la modificación de los LEDs y valores mostrados en el LCD. */
bool HOLD = false;
/*==================[internal functions declaration]=========================*/
/** @brief Inicializa los componentes del sistema. */
void initComponentes(void) {
	static neopixel_color_t color;
	SwitchesInit();
	HcSr04Init(GPIO_3, GPIO_2); // Initialize HC-SR04 with GPIO_3 as Echo and GPIO_2 as Trigger
	NeoPixelInit(BUILT_IN_RGB_LED_PIN, BUILT_IN_RGB_LED_LENGTH, &color);
    NeoPixelAllOff();
}

/**
 * @brief Tarea muestra el resultado del valor medido de _distancia_.
 * @details Muestra el resultado en el LCD y lo representa en los LEDs según el diseño planteado.
 */
void encenderLedsSegunDistancia(uint16_t distancia) {
	// encender o apagar LEDs segun distancia.
	if (distancia < 10) {
		NeoPixelAllOff();
		#if DEBUG_MODE
			printf("Estado LEDs: 0 0 0\n"); // Print LEDs status
		#endif
	} else if (distancia < 20) {
		NeoPixelAllColor(NEOPIXEL_COLOR_RED);
		#if DEBUG_MODE
			printf("Estado LEDs: 1 0 0\n"); // Print LEDs status
		#endif
	} else if (distancia < 30) {
		NeoPixelAllColor(NEOPIXEL_COLOR_YELLOW);
		#if DEBUG_MODE
			printf("Estado LEDs: 1 1 0\n"); // Print LEDs status
		#endif
	} else {
		NeoPixelAllColor(NEOPIXEL_COLOR_GREEN);
		#if DEBUG_MODE
			printf("Estado LEDs: 1 1 1\n"); // Print LEDs status
		#endif
	}
}

/**
 * @brief Tarea que mide la distancia, muestra el resultado en el LCD y enciende los LEDs.
*/
static void medirMostrarTask(void *pvParameter){
	while (true)
	{
		uint16_t distancia = 0;
		if (MEDIR == true) {
			//distancia = 1 + rand() % 39; // Simulate a distance between 1 and 40 cm
			distancia = HcSr04ReadDistanceInCentimeters();
			#if DEBUG_MODE
				printf("Distancia: %u cm\n", distancia); // Print the measured distance
				// printf("Estado... \n");
				// printf("\tMEDIR: %d\n", MEDIR); // Print the state of MEDIR
				// printf("\tHOLD: %d\n", HOLD); // Print the state of HOLD
			#endif
		}
		if (HOLD == false) {
			encenderLedsSegunDistancia(distancia);
			LcdItsE0803Write(distancia);
			#if DEBUG_MODE
				printf("LCD: %u cm\n", distancia);
			#endif
		}
		#if DEBUG_MODE
			printf("Fin medicion\n"); // Print end of measurement message
		#endif

		vTaskDelay(DELAY_MEDICION / portTICK_PERIOD_MS); // Delay for 1 second
		
		#if DEBUG_MODE
			printf("Fin delay de 1 segundo\n"); // Print delay message
		#endif
	}
}

/**
 * @brief Tarea que lee las teclas y cambia el estado de MEDIR y HOLD.
 * @details Cambia el estado de MEDIR y HOLD según la tecla presionada.
 */
static void teclasTask(void *pvParameter){
	while (true) {
		if (SwitchesRead() == TEC1) {
			MEDIR = !MEDIR;
			#if DEBUG_MODE
				printf("\nToggled MEDIR from: %d --> %d", !MEDIR, MEDIR);
			#endif
			if (MEDIR == false)	{
				LcdItsE0803Off();	// Apagar el LCD
				encenderLedsSegunDistancia(1);	// Apagar todos los LEDs
			}
		}
		if (SwitchesRead() == TEC2 && MEDIR == 1) {
			HOLD = !HOLD;			#if DEBUG_MODE
				printf("Toggled MEDIR from: %d --> %d", !MEDIR, MEDIR);
			#endif
			#if DEBUG_MODE
				printf("\nToggled HOLD from: %d --> %d", !HOLD, HOLD);
			#endif
		}
		vTaskDelay(DELAY_TASK / portTICK_PERIOD_MS); // Delay for 100 ms
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){
	initComponentes();
	uint32_t stack_size = 1024;
	#if DEBUG_MODE
		stack_size = 4096; // Increase stack size por los printf()s de depuración
	#endif
	xTaskCreate(&medirMostrarTask, "MEDIRMOSTRAR", stack_size, NULL, 5, &medirMostrarTask_handle);
	xTaskCreate(&teclasTask, "TECLAS", stack_size, NULL, 5, &teclasTask_handle);
}
/*==================[end of file]============================================*/