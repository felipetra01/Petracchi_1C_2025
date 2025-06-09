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
 * Además se debe monitorizar la radiación ambiente, para ello se cuenta con un sensor analógico que da una
 * salida de 0V para 0mR/h y 3.3V para una salida de 100 mR/h. Se deben tomar muestras de radiación cada 5
 * segundos, enviando el mensaje por UART : “Radiación 30mR/h”
 * Si se sobrepasan los 40mR/h se debe informar del riesgo por Radiación, encendiendo el led Amarillo de la
 * placa, y enviando en el mensaje: “Radiación 50mR/h - RADIACIÓN ELEVADA”
 * Si no hay riesgo de nevada ni radiación excesiva, se indicará con el led Verde esta situación.
 * 
 * <a href=" ">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * |  DHT11 Data	| 	GPIO_23		|
 * |  LED_3 (Rojo)	| 	GPIO_5		|
 * |LED_2 (Amarillo)| 	GPIO_10		|
 * |  LED_1 (Verde)	| 	GPIO_11		| 
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
#include "uart_mcu.h"
#include "string.h"
#include "led.h"

#include "dht11.h"
/*==================[macros and definitions]=================================*/
#define LED_ROJO LED_3
#define LED_AMARILLO LED_2
#define LED_VERDE LED_1
#define BAUD_RATE 19200

/*==================[internal data definition]===============================*/
TaskHandle_t medirEnviarTyHTask_handle = NULL;
TaskHandle_t medirEnviarRadTask_handle = NULL;

float temperatura = 0.0f;
float humedad = 0.0f;
bool riesgoNevada = false;
bool radiacionElevada = false;

/*==================[internal functions declaration]=========================*/
void initComponentes(void) {
	LedsInit();
	dht11Init(GPIO_23);
}

void medirEnviarTyH(void *param) {
	vTaskNotifyGiveFromISR(medirEnviarTyHTask_handle, pdFALSE);
}

void medirEnviarTyHTask(void *param) {
	while (true) {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (dht11Read(&humedad, &temperatura)) {
			// Si la lectura es exitosa, verifica las condiciones de riesgo de nevada
			if (humedad > 85.0f && temperatura >= 0.0f && temperatura <= 2.0f) {
				riesgoNevada = true;
				UartSendTyH(temperatura, humedad, "RIESGO DE NEVADA");
			} else {
				riesgoNevada = false;
			}
		}
		encenderLeds();
		UartSendTyH(temperatura, humedad, NULL); // Envía los datos por UART
	}
}

void UartSendTyH(float temp, float hum, char* flag) {
	char str[60];
	sprintf(str, "Temperatura: %.1fºC - Húmedad: %.1f%%", temp, hum);
	if (flag != NULL) {
		strcat(str, " - ");
		strcat(str, flag);
	}
	UartSendString(UART_PC, str);
	UartSendString(UART_PC, " in\r\n");
}

void medirEnviarRad(void *param) {
	vTaskNotifyGiveFromISR(medirEnviarRadTask_handle, pdFALSE);
}

void medirEnviarRadTask(void *param) {
	while (true) {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		float radiacion = 30.0f; // Simula una lectura de radiación en mR/h

		if (radiacion > 40.0f) {
			radiacionElevada = true;
			UartSendRad(radiacion, "RADIACIÓN ELEVADA");
		} else {
			radiacionElevada = false;
		}
		encenderLeds();
		UartSendRad(radiacion, NULL);
	}
}

void UartSendRad(float rad, char* flag) {
	char str[60];
	sprintf(str, "Radiación: %.1f mR/h", rad);
	if (flag != NULL) {
		strcat(str, " - ");
		strcat(str, flag);
	}
	UartSendString(UART_PC, str);
	UartSendString(UART_PC, " in\r\n");
}

void encenderLeds(void ) {
	if (riesgoNevada) {
		LedOn(LED_ROJO);
		LedOff(LED_AMARILLO);
		LedOff(LED_VERDE);
	} else if (radiacionElevada) {
		LedOff(LED_ROJO);
		LedOn(LED_AMARILLO);
		LedOff(LED_VERDE);
	} else {
		LedOff(LED_ROJO);
		LedOff(LED_AMARILLO);
		LedOn(LED_VERDE);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	initComponentes();
	serial_config_t uart_config = {
		.port = UART_PC,
		.baud_rate = BAUD_RATE,
		.func_p = NULL,
		.param_p = NULL,
	};
	UartInit(&uart_config);

	timer_config_t timer_medirEnviarTyH_config = {
		.timer = TIMER_A,		// Timer A
		.period = 1000 * 1000,	// 1 segundo en microsegundos
		.func_p = medirEnviarTyH,		// Función a ejecutar
		.param_p = NULL			// Parámetro para la función
	};
	TimerInit(&timer_medirEnviarTyH_config);

	timer_config_t timerMedirEnviarRad_config = {
		.timer = TIMER_B,		// Timer B
		.period = 5000 * 1000,	// 5 segundos en microsegundos
		.func_p = medirEnviarRad,		// Función a ejecutar
		.param_p = NULL			// Parámetro para la función
	};
	TimerInit(&timerMedirEnviarRad_config);

	xTaskCreate(&medirEnviarTyHTask, "MEDIRTYH", 1024, NULL, 5, &medirEnviarTyHTask_handle);
	xTaskCreate(&medirEnviarRadTask, "MEDIRRAD", 1024, NULL, 5, &medirEnviarRadTask_handle);

	TimerStart(timer_medirEnviarTyH_config.timer);
	TimerStart(timerMedirEnviarRad_config.timer);
}
/*==================[end of file]============================================*/