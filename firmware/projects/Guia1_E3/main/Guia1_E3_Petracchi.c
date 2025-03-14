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
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
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
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 100
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
struct leds
{
	uint8_t mode;       // ON (1), OFF (2), TOGGLE
	uint8_t n_led;      // indica el número de led a controlar
	uint8_t n_ciclos;   // indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;   // indica el tiempo de cada ciclo
};
/*==================[external functions definition]==========================*/
void app_main(void){
	/*
	struct leds leds[] = {
		{ON, LED_1, 5, 100},
		{OFF, LED_2, 10, 200},
		{TOGGLE, LED_3, 15, 300}
	};
	*/
	struct leds estado1 = {1, LED_1, 5, 100};
	if (estado1.mode == 1)	// ON
	{
		if (estado1.n_led == 1) 	 { LedOn(LED_1); }
		else if (estado1.n_led == 2) { LedOn(LED_2); }
		else if (estado1.n_led == 3) { LedOn(LED_3); }
	}
	else if (estado1.mode == 2)		// OFF
	{
		if (estado1.n_led == 1) 	 { LedOff(LED_1); }
		else if (estado1.n_led == 2) { LedOff(LED_2); }
		else if (estado1.n_led == 3) { LedOff(LED_3); }
	}
	else if (estado1.mode == 3)		// TOGGLE (no terminado)
	{
		for (uint8_t i = 0; i < estado1.n_ciclos; i++)
		{
			LedToggle(estado1.n_led);
			vTaskDelay(estado1.periodo / portTICK_PERIOD_MS);
		}
	}
}
/*==================[end of file]============================================*/