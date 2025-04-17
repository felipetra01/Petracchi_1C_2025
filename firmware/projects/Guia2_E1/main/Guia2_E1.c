/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Diseñar el firmware modelando con un diagrama de flujo de manera que cumpla con las siguientes funcionalidades:
 * 
 * 1) Mostrar distancia medida utilizando los leds de la siguiente manera:
 * Si la distancia es menor a 10 cm, apagar todos los LEDs.
 * Si la distancia está entre 10 y 20 cm, encender el LED_1.
 * Si la distancia está entre 20 y 30 cm, encender el LED_2 y LED_1.
 * Si la distancia es mayor a 30 cm, encender el LED_3, LED_2 y LED_1.
 * 2) Mostrar el valor de distancia en cm utilizando el display LCD.
 * Usar TEC1 para activar y detener la medición.
 * Usar TEC2 para mantener el resultado (“HOLD”).
 * Refresco de medición: 1 s
 * Se deberá conectar a la EDU-ESP un sensor de ultrasonido HC-SR04 y una pantalla LCD y utilizando los drivers
 * provistos por la cátedra implementar la aplicación correspondiente. Se debe subir al repositorio el código.
 * Se debe incluir en la documentación, realizada con doxygen, el diagrama de flujo. 
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
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
/*==================[macros and definitions]=================================*/
#define DELAY_TASK 100
#define DELAY_MEDICION 1000
#define TEC1 SWITCH_1
#define TEC2 SWITCH_2
/*==================[internal data definition]===============================*/
TaskHandle_t medirMostrarTask_handle = NULL;
TaskHandle_t teclasTask_handle = NULL;

bool MEDIR = true;
bool HOLD = false;
/*==================[internal functions declaration]=========================*/
void initComponentes(void) {
	LedsInit();
	SwitchesInit();
	HcSr04Init(GPIO_3, GPIO_2); // Initialize HC-SR04 with GPIO_3 as Echo and GPIO_2 as Trigger
}

static void medirMostrarTask(void *pvParameter){
	uint16_t distancia = 0;
	if (MEDIR == true) {
		distancia = HcSr04ReadDistanceInCentimeters();
		if (distancia < 10) {
			LedOff(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		} else if (distancia < 20) {
			LedOn(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		} else if (distancia < 30) {
			LedOn(LED_1);
			LedOn(LED_2);
			LedOff(LED_3);
		} else {
			LedOn(LED_1);
			LedOn(LED_2);
			LedOn(LED_3);
		}
	}
	if (HOLD == false) {
		LcdItsE0803Write(distancia);		
	}
	vTaskDelay(DELAY_MEDICION / portTICK_PERIOD_MS); // Delay for 1 second
}

static void teclasTask(void *pvParameter){
	while (true) {
		if (SwitchesRead() == TEC1) {
			MEDIR = !MEDIR;
			LcdItsE0803Off();
		}
		if (SwitchesRead() == TEC2 && MEDIR == 1) {
			HOLD = !HOLD;
		}
		vTaskDelay(DELAY_TASK / portTICK_PERIOD_MS); // Delay for 100 ms
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){
	initComponentes();
	xTaskCreate(&medirMostrarTask, "MEDIRMOSTRAR", 512, NULL, 5, &medirMostrarTask_handle);
	xTaskCreate(&teclasTask, "TECLAS", 512, NULL, 5, &teclasTask_handle);
}
/*==================[end of file]============================================*/