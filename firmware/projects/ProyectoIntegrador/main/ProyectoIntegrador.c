/*! @mainpage Proyecto Integrador
 *
 * @section genDesc General Description
 *
 * 
 *
 * <a href=" ">Ejemplo de operación</a>
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
 * | XX/0X/2025 | Document creation		                         |
 *
 * @author Felipe M. Petracchi (felipe.petracchi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"

#include "neopixel_stripe.h"
#include "ble_mcu.h"
#include "led.h"
/*==================[macros and definitions]=================================*/
#define DEBUG_PRINTS 0

TaskHandle_t bleStatusCheckTask_handle = NULL;

uint8_t cantPulsos;		// [0, 100] --> Cantidad de veces a hacer PWMOn y PWMOff

uint8_t durPulso;
uint8_t durPausa;		
uint8_t instensidad;	// [0, 100] --> Duty cycle en %

uint16_t distancia = 0;	// [0, 1000] --> Distancia en cm
uint8_t distGiro = 0;
char dirGiro = '\0';
uint8_t distFinal = 0;

uint8_t motorNro = 0;	// [0, 2] --> Motor a controlar (0: Izq, 2: Adelante, 1: Der)
enum {LEFT_MOTOR, RIGHT_MOTOR, FWD_MOTOR} motor;

/*==================[internal data definition]===============================*/

void hacerAviso(void *param){
	cantPulsos = distancia / 10;
	if (cantPulsos > 1) {
		// vibrateNTimes(cantPulsos, PWM_0);			// Esto, o
		// vibrateNTimesLongEnd(cantPulsos);	// Para más de 10 pulsos vibrar largo al final
	} else {
		// vibrateLong();
	}
}

void bleStatusCheck(void *param){
	vTaskNotifyGiveFromISR(bleStatusCheckTask_handle, pdFALSE);
}

void bleStatusCheckTask(void *param){
	while (1) {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        switch(BleStatus()){
            case BLE_OFF:
    			NeoPixelAllColor(NeoPixelRgb2Color(255, 0, 0));
				LedsOffAll();
				LedOn(LED_3);
            break;
            case BLE_DISCONNECTED:
				NeoPixelAllColor(NeoPixelRgb2Color(0, 0, 255));
				LedsOffAll();
				LedOn(LED_2);
            break;
            case BLE_CONNECTED:
				NeoPixelAllColor(NeoPixelRgb2Color(0, 255, 0));
				LedsOffAll();
				LedOn(LED_1);
            break;
        }
	}
}
/*==================[internal functions declaration]=========================*/
void read_data(uint8_t * data, uint8_t length){

}
/*==================[external functions definition]==========================*/
void app_main(void){
	neopixel_color_t color = NeoPixelRgb2Color(0, 0, 0);
	NeoPixelInit(BUILT_IN_RGB_LED_PIN, BUILT_IN_RGB_LED_LENGTH, &color);
	NeoPixelBrightness(100);

    ble_config_t ble_configuration = {
        "ESP_EDU_Feli",
        read_data
    };
	BleInit(&ble_configuration);
	LedsInit();
	timer_config_t bleStatusCheckTimer = {
        .timer = TIMER_A,
        .period = 200 * 1000, // 200 ms in microseconds
        .func_p = bleStatusCheck,
        .param_p = NULL
    };
	TimerInit(&bleStatusCheckTimer);

	xTaskCreate(bleStatusCheckTask, "BleStatusCheckTask", 2048, NULL, 5, &bleStatusCheckTask_handle);
	TimerStart(bleStatusCheckTimer.timer);
}
/*==================[end of file]============================================*/