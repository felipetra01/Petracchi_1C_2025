/*! @mainpage Examen
 *
 * @section genDesc General Description
 *
 * El sistema está compuesto por un sensor de humedad y temperatura DHT11 y un  sensor analógico de radiación.
 * 
 * El firmware implementa las siguientes funcionalidades:
 * 1. Medición periódica de temperatura y humedad utilizando el sensor DHT11.
 * 2. Envío de los datos de temperatura y humedad por UART a un terminal en la PC.
 * 3. Medición periódica de radiación y envío de los datos por UART.
 * 4. Encendido de LEDs según las condiciones de riesgo.
 * 5. Gestión de dos teclas (botones) para iniciar y detener las mediciones:
 *   - Tecla 1 (TEC1): Inicia las mediciones de temperatura, humedad y radiación.
 *   - Tecla 2 (TEC2): Detiene las mediciones y apaga los LEDs.
 * 
 * 
 * <a href=" ">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral    |     ESP32  			|
 * |:----------------:|:--------------------|
 * |  DHT11 Data	  | 	GPIO_23			|
 * |  Sensor Rad.	  | 	GPIO_1 (CH_1) 	|
 * |  LED_3 (Rojo)	  | 	GPIO_5			|
 * |  LED_2 (Amarillo)| 	GPIO_10			|
 * |  LED_1 (Verde)	  | 	GPIO_11			| 
 * |  Switch 1 (TEC1) | 	GPIO_4			|
 * |  Switch 2 (TEC2) | 	GPIO_15			|
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
#include "switch.h"
#include "analog_io_mcu.h"

#include "dht11.h"
/*==================[macros and definitions]=================================*/
#define LED_ROJO LED_3
#define LED_AMARILLO LED_2
#define LED_VERDE LED_1
#define ADC_CHANNEL CH1

#define MEDIR_TYH_PERIOD_MS 1000 // Periodo de muestreo de temperatura y humedad
#define MEDIR_RAD_PERIOD_MS 5000 // Periodo de muestreo de radiación
#define BAUD_RATE 19200


/*==================[internal data definition]===============================*/
/*
 * @brief Task handle para la tarea medirEnviarTyHTask
 * @note Esta tarea se encarga de medir la temperatura y humedad, y enviar los datos por UART.
*/
TaskHandle_t medirEnviarTyHTask_handle = NULL;
/*
 * @brief Task handle para la tarea medirEnviarRadTask
 * @note Esta tarea se encarga de medir la radiación y enviar los datos por UART.
*/
TaskHandle_t medirEnviarRadTask_handle = NULL;

/*
 * @brief Task handle para la tarea teclasTask
 * @note Esta tarea se encarga de gestionar las teclas y notificar a la tarea correspondiente.
*/
TaskHandle_t teclasTask_handle = NULL;

float temperatura = 0.0f;
float humedad = 0.0f;
float radiacion = 0.0f;
bool riesgoNevada = false;
bool radiacionElevada = false;

/*==================[internal functions declaration]=========================*/
/*
 * @brief Inicializa los componentes del sistema.
 * @details Configura los LEDs y el sensor DHT11.
 */
void initComponentes(void) {
	LedsInit();
	dht11Init(GPIO_23);
	switchesInit(); // Inicializa los switches (botones)
}

/*
 * @brief Función que se ejecuta cuando el timer A genera una interrupción.
 * @note Esta función se encarga de notificar a la tarea medirEnviarTyHTask.
 */
void medirEnviarTyH(void *param) {
	vTaskNotifyGiveFromISR(medirEnviarTyHTask_handle, pdFALSE);
}

/*
 * @brief Tarea que mide la temperatura y humedad, y envía los datos por UART.
 * @note Esta tarea se ejecuta periódicamente y lee los valores del sensor DHT11.
 */
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
				UartSendTyH(temperatura, humedad, NULL); // Envía los datos por UART
			}
		}
		encenderLeds();
	}
}

/*
 * @brief Envía la temperatura y humedad por UART.
 * @param temp Temperatura medida.
 * @param hum Humedad medida.
 * @param flag Indicador de riesgo (opcional).
 * @note Si el flag es NULL, no se envía información adicional.
*/
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

/*
 * @brief Función que se ejecuta cuando el timer B genera una interrupción.
 * @note Esta función se encarga de notificar a la tarea medirEnviarRadTask.
*/
void medirEnviarRad(void *param) {
	vTaskNotifyGiveFromISR(medirEnviarRadTask_handle, pdFALSE);
}

/*
 * @brief Tarea que mide la radiación y envía los datos por UART.
 * @note Esta tarea se ejecuta periódicamente y simula una lectura de radiación.
*/
void medirEnviarRadTask(void *param) {
	while (true) {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		uint16_t analog_value = 0;
        AnalogInputReadSingle(ADC_CHANNEL, &analog_value);

		radiacion = (float)analog_value * (1/3345.0f) * 100.0f; // Convertir a mR/h

		if (radiacion > 40.0f) {
			radiacionElevada = true;
			UartSendRad(radiacion, "RADIACIÓN ELEVADA");
		} else {
			radiacionElevada = false;
			UartSendRad(radiacion, NULL);
		}
		encenderLeds();
	}
}

/*
 * @brief Envía la radiación medida por UART.
 * @param rad Radiación medida en mR/h.
 * @param flag Indicador de riesgo (opcional).
 * @note Si el flag es NULL, no se envía información adicional.
*/
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

/*
 * @brief Enciende los LEDs según el estado de riesgo de nevada y radiación.
 * @note Si hay riesgo de nevada, enciende el LED rojo; si hay radiación elevada, enciende el LED amarillo;
 * de lo contrario, enciende el LED verde.
*/
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

void teclasFunc(void) {
	vTaskNotifyGiveFromISR(medirEnviarTyHTask_handle, pdFALSE);
}
void teclasTask(void *pvParameter) {
	while (true) {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(SwitchesRead() == SWITCH_1) {
			if(TimerRead(TIMER_A) == 0 && TimerRead(TIMER_B) == 0) {
				TimerStart(TIMER_A);	// Timer A de medirEnviarTyH
				TimerStart(TIMER_B);	// Timer B de medirEnviarRad
			}
		}
		else if(SwitchesRead() == SWITCH_2) {
			TimerStop(TIMER_A);		// Timer A de medirEnviarTyH
			TimerStop(TIMER_B);		// Timer B de medirEnviarRad
			LedsOffAll();
		}
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
		.period = MEDIR_TYH_PERIOD_MS * 1000,	// 1 segundo en microsegundos
		.func_p = medirEnviarTyH,		// Función a ejecutar
		.param_p = NULL			// Parámetro para la función
	};
	TimerInit(&timer_medirEnviarTyH_config);

	timer_config_t timerMedirEnviarRad_config = {
		.timer = TIMER_B,		// Timer B
		.period = MEDIR_RAD_PERIOD_MS * 1000,	// 5 segundos en microsegundos
		.func_p = medirEnviarRad,		// Función a ejecutar
		.param_p = NULL			// Parámetro para la función
	};
	TimerInit(&timerMedirEnviarRad_config);

	timer_config_t timer_teclas_config = {
		.timer = TIMER_C,		// Timer C
		.period = 20 * 1000,	// 20 ms en microsegundos
		.func_p = teclasFunc,	// Función a ejecutar
		.param_p = NULL			// Parámetro para la función
	};
	TimerInit(&timer_teclas_config);

	xTaskCreate(&medirEnviarTyHTask, "MEDIRTYH", 1024, NULL, 5, &medirEnviarTyHTask_handle);
	xTaskCreate(&medirEnviarRadTask, "MEDIRRAD", 1024, NULL, 5, &medirEnviarRadTask_handle);
	xTaskCreate(&teclasTask, "TECLAS", 1024, NULL, 5, &teclasTask_handle);
	
	// TimerStart(timer_medirEnviarTyH_config.timer);
	// TimerStart(timerMedirEnviarRad_config.timer);
	TimerStart(timer_teclas_config.timer);
}
/*==================[end of file]============================================*/