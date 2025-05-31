/*! @mainpage Guia 2 Actividad 2 - Petracchi, F. M.
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

/*==================[macros and definitions]=================================*/
/**
 * @def DEBUG_MODE
 * @brief Modo de depuración
 * @details 1 para habilitar "printf()"s de depuración, 0 para deshabilitar.
 */
#define DEBUG_MODE 0
/**
 * @def DELAY_MEDICION
 * @brief Delay que marca la tasa de refresco de la tarea de medición y muestra.
 * @details 1000 ms
 */
#define DELAY_MEASURE 300
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

/*==================[internal data definition]===============================*/
/**
 * @brief identificador de tipo TaskHandle_t de la tarea "medirMostrar".
 * @details Se utiliza para la creación de la tarea y para su control. */
TaskHandle_t measureAndShow_task_handle = NULL;

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
	LedsInit();
	SwitchesInit();
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();
}

/**
 * @brief Tarea muestra el resultado del valor medido de _distancia_.
 * @details Muestra el resultado en el LCD y lo representa en los LEDs según el diseño planteado.
 */
void encenderLedsSegunDistancia(uint16_t distancia) {
	// encender o apagar LEDs segun distancia.
	if (distancia < 10) {
		LedOff(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
		#if DEBUG_MODE
			printf("Estado LEDs: 0 0 0\n"); // Print LEDs status
		#endif
	} else if (distancia < 20) {
		LedOn(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
		#if DEBUG_MODE
			printf("Estado LEDs: 1 0 0\n"); // Print LEDs status
		#endif
	} else if (distancia < 30) {
		LedOn(LED_1);
		LedOn(LED_2);
		LedOff(LED_3);
		#if DEBUG_MODE
			printf("Estado LEDs: 1 1 0\n"); // Print LEDs status
		#endif
	} else {
		LedOn(LED_1);
		LedOn(LED_2);
		LedOn(LED_3);
		#if DEBUG_MODE
			printf("Estado LEDs: 1 1 1\n"); // Print LEDs status
		#endif
	}
}

void measureAndShow(void *param) {
	vTaskNotifyGiveFromISR(measureAndShow_task_handle, pdFALSE);
}

/**
 * @brief Tarea que mide la distancia, muestra el resultado en el LCD y enciende los LEDs.
*/
static void measureAndShowTask(void *pvParameter){
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

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
		if (HOLD == false && MEDIR == true) {
			encenderLedsSegunDistancia(distancia);
			LcdItsE0803Write(distancia);
			#if DEBUG_MODE
			printf("LCD: %u cm\n", distancia);
			#endif
		}
	}
}

void if_TEC1_do(){
	MEDIR = !MEDIR;
	if (MEDIR == false)	{
		LcdItsE0803Off();				// Apagar el LCD
		encenderLedsSegunDistancia(1);	// Apagar todos los LEDs
	}
	#if DEBUG_MODE
		printf("\nToggled MEDIR from: %d --> %d\n", !MEDIR, MEDIR);
	#endif
}

void if_TEC2_do(){
	if (MEDIR) {
		HOLD = !HOLD;
		#if DEBUG_MODE
			printf("\nToggled HOLD from: %d --> %d\n", !MEDIR, MEDIR);
		#endif
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	initComponentes();

    /* Inicialización de timers */
    timer_config_t timer_measure = {
        .timer = TIMER_A,
        .period = DELAY_MEASURE * 1000,
        .func_p = measureAndShow,
        .param_p = NULL
    };
	TimerInit(&timer_measure);

	uint32_t stack_size = 1024;
	#if DEBUG_MODE
		stack_size = 8184; // Increase stack size por los printf()s de depuración
	#endif
	
	SwitchActivInt(SWITCH_1, if_TEC1_do, NULL);
	SwitchActivInt(SWITCH_2, if_TEC2_do, NULL);
	
	xTaskCreate(&measureAndShowTask, "MEASUREANDSHOW", stack_size, NULL, 5, &measureAndShow_task_handle);
	TimerStart(timer_measure.timer);
}
/*==================[end of file]============================================*/