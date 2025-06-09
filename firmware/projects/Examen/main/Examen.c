/*! @mainpage Examen
 *
 * @section genDesc General Description
 *
 * El sistema está compuesto por un sensor de humedad y temperatura DHT11 y un  sensor analógico de radiación.
 * 
 * Por un lado se debe detectar el riesgo de nevada, la cual se da si la húmedad se encuentra por encima del
 * 85%  y la temperatura entre 0 y 2ºC. Para esto se deben tomar muestras cada 1 segundo y se envían por UART
 * con el siguiente formato: “Temperatura: 10ºC - Húmedad: 70%”
 * Si se da la condición de riesgo de nevada se debe indicar el estado encendiendo el led Rojo de la placa,
 * además del envío de un mensaje por la UART : * “Temperatura: 1ºC - Húmedad: 90% - RIESGO DE NEVADA”
 *
 * <a href=" ">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	DHT11 Data	| 	GPIO_4		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 09/06/2025 | Document creation		                         |
 *
 * @author Felipe M. Petracchi (felipe.petracchi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"

#include "dht11.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
TaskHandle_t medirTyHTask_handle = NULL; // Identificador de la tarea de medición y envío de datos

float temperatura = 0.0f;
float humedad = 0.0f;

/*==================[internal functions declaration]=========================*/
void initComponentes(void) {
	LedsInit();
	dht11Init(GPIO_23); // Inicializa el DHT11 en el GPIO 4
}

void medirTyH(void *param) {
	vTaskNotifyGiveFromISR(medirTyHTask_handle, pdFALSE);
}

void medirTyHTask(void *param) {
	while (true) {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Espera a que se notifique la tarea

		if (dht11Read(&humedad, &temperatura)) {
			printf("Temperatura: %.1fºC - Húmedad: %.1f%%\n", temperatura, humedad);
		}
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){
	initComponentes();

	timer_config_t timer_medirTyH_config = {
		.timer = TIMER_A,		// Timer A
		.period = 1000 * 1000,	// 1 segundo en microsegundos
		.func_p = medirTyH,		// Función a ejecutar
		.param_p = NULL			// Parámetro para la función
	};

	xTaskCreate(&medirTyHTask, "MEDIRTYH", 1024, NULL, 5, &medirTyHTask_handle);

	TimerStart(timer_medirTyH_config.timer);
}
/*==================[end of file]============================================*/