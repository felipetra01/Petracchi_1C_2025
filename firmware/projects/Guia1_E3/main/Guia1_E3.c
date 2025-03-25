/*! @mainpage Blinking switch
 *
 * \section genDesc General Description
 *
 * This example makes LED_1 and LED_2 blink if SWITCH_1 or SWITCH_2 are pressed.
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
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 100
#define ON 1
#define OFF 2
#define TOGGLE 3
/*==================[internal data definition]===============================*/
struct leds
{
	uint8_t mode;       // ON (1), OFF (2), TOGGLE
	uint8_t n_led;      // indica el número de led a controlar
	uint8_t n_ciclos;   // indica la cantidad de ciclos de encendido/apagado
	uint16_t periodo;   // indica el tiempo entre cada toggle del ciclo (t_on o t_off)
};
/*==================[internal functions declaration]=========================*/
void EncenderEstado (struct leds *estado) {
	if (estado->mode == ON) { LedOn(estado->n_led); }
	else if (estado->mode == OFF) { LedOff(estado->n_led); }
	else if (estado->mode == TOGGLE)
	{
		for (uint8_t i = 0; i < estado->n_ciclos*2; i++)
		{
			LedToggle(estado->n_led);
			for (uint8_t j = 0; j < ((estado->periodo) / CONFIG_BLINK_PERIOD); j++)	{ vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);	}
		}
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){

	LedsInit();

	struct leds estado1 = {ON, LED_1, 5, 500};
	struct leds estado2 = {OFF, LED_1, 5, 300};
	struct leds estado3 = {TOGGLE, LED_3, 10, 500};
	
	EncenderEstado(&estado1);
	vTaskDelay(estado1.periodo / portTICK_PERIOD_MS);
	EncenderEstado(&estado2);
	EncenderEstado(&estado3);
}