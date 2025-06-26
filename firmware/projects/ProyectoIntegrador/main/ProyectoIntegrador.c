/*! @mainpage Proyecto Integrador
 *
 * @section genDesc General Description
 *
 * Se ha diseñado un dispositivo wearable basado en un microcontrolador ESP32-C6 que, junto con una aplicación
 * Android, conforma una solución integrada de asistencia háptica. El ESP32-C6 recibe mediante Bluetooth datos
 * de navegación extraídos en tiempo real por la app, traduce las instrucciones en comandos de control y activa
 * tres motores vibradores montados en una muñequera o cinturón flexible. Cada vibrador indica una dirección
 * específica (adelante, izquierda, derecha), mientras que dos teclas físicas permiten silenciar la señal o
 * cambiar el modo de aviso (distancia al próximo giro o distancia restante al destino).

 * <a href=" ">Ejemplo de operación</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	Left Motor 	| 	GPIO_11		|
 * | 	Right Motor | 	GPIO_10		|
 * |	Fwd. Motor	| 	GPIO_5		|
 * | 	Motors Gnd	| 	GND			|
 * | 	Motors Vcc	| 	VCC(5V)		|
 * |   NeoPixel LED	| 	GPIO_8		|
 * | 	Switch 1	| 	GPIO_4		|
 * | 	Switch 2	| 	GPIO_15		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 22/05/2025 | Document creation		                         |
 *
 * @author Felipe M. Petracchi (felipe.petracchi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
#include "timer_mcu.h"

#include "neopixel_stripe.h"
#include "ble_mcu.h"
#include "vibration_motor.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define DEBUG_PRINTS 0
#define LEFT_MOTOR MOTOR_0
#define RIGHT_MOTOR MOTOR_1
#define FWD_MOTOR MOTOR_2
/*==================[internal data definition]===============================*/
TaskHandle_t bleStatusCheckTask_handle = NULL;
TaskHandle_t hacerAvisoTask_handle = NULL;

uint16_t durPulso = 300;
uint16_t durPausa = 200;
uint8_t instensidad = 90;			// [0, 100] --> Duty cycle en %
uint16_t intervaloAvisos_MS = 4000; // Intervalo entre avisos en ms

uint8_t cantPulsos = 0; // [0, 100] --> Cantidad de veces a hacer PWMOn y PWMOff
uint16_t distancia = 0; // [0, 1000] --> Distancia en cm
uint8_t distGiro = 0;
uint8_t distFinal = 0;
char dirGiro = '\0';
uint8_t motorNro = 0; // [0, 2] --> Motor a controlar (0: Izq, 2: Adelante, 1: Der)

bool deviceConnected = false;
bool boolHacerAviso = true;
bool boolAvisarDistFinal = false;

/*==================[internal functions declaration]=========================*/
/*
 * @brief Función que notifica a la tarea de verificación del estado del BLE.
*/
void bleStatusCheck(void *param)
{
	vTaskNotifyGiveFromISR(bleStatusCheckTask_handle, pdFALSE);
}

/*
 * @brief Tarea que verifica el estado del BLE y actualiza el color del NeoPixel según el estado de conexión.
 *
 * @param param Parámetro de la tarea (no se utiliza en este caso).
 * @note Esta tarea se ejecuta indefinidamente pero espera una notificación para ejecutarse cada vez.
 *       Cambia el color del NeoPixel a rojo si está apagado, azul si está desconectado y verde si está conectado.
*/
void bleStatusCheckTask(void *param)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		switch (BleStatus())
		{
		case BLE_OFF:
			NeoPixelAllColor(NeoPixelRgb2Color(255, 0, 0));
			deviceConnected = false;
			break;
		case BLE_DISCONNECTED:
			NeoPixelAllColor(NeoPixelRgb2Color(0, 0, 255));
			deviceConnected = false;
			break;
		case BLE_CONNECTED:
			NeoPixelAllColor(NeoPixelRgb2Color(0, 255, 0));
			deviceConnected = true;
			break;
		}
	}
}

/*
 * @brief Función que lee los datos recibidos por BLE y los procesa.
 *
 * @param data Puntero a los datos recibidos.
 * @param length Longitud de los datos recibidos.
 * @note Se espera que los datos tengan el formato "distGiro-dirGiro-distFinal".
 *       Si el formato es válido, se extraen las variables distGiro, dirGiro y distFinal.
*/
void read_data(uint8_t *data, uint8_t length)
{
	if (length >= 7 && length < 32)
	{					 // mínimo esperable: "1-A-1" (5), típico: "20-R-130" (8)
		char buffer[32]; // buffer temporal para asegurar string terminada en null
		memcpy(buffer, data, length);
		buffer[length] = '\0'; // Agregar terminador nulo

		int n = sscanf(buffer, "%hhu-%c-%hhu", &distGiro, &dirGiro, &distFinal);
		if (n == 3)
		{
			#if DEBUG_PRINTS
				printf("\nLectura - distGiro: %d, direcc: %c, distFinal: %d\n", distGiro, dirGiro, distFinal);
			#endif
		}
	} else {
	#if DEBUG_PRINTS
		printf("⚠️ Formato de mensaje inválido:");
		for (uint8_t i = 0; i < length; i++)
		{
			printf("%c", data[i]);
		}
		printf("\n");
	#endif
	}
}

/*
 * @brief Función que se llama desde el temporizador para hacer un aviso.
 *
 * @param param Parámetro de la función (no se utiliza en este caso).
 * @note Esta función notifica a la tarea de hacer aviso para que realice la acción correspondiente.
*/
void hacerAviso(void *param)
{
	vTaskNotifyGiveFromISR(hacerAvisoTask_handle, pdFALSE);
}

void hacerAvisoTask(void *param)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, intervaloAvisos_MS / portTICK_PERIOD_MS);
		// ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (!boolHacerAviso)
		{
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		}

		if (deviceConnected && boolHacerAviso)
		{
			MotorOff(LEFT_MOTOR);
			MotorOff(RIGHT_MOTOR);
			MotorOff(FWD_MOTOR);

			if (boolAvisarDistFinal) {
				distancia = distFinal;
			} else {
				distancia = distGiro;
			}
			cantPulsos = distancia / 10;
			// if (!boolAvisarDistFinal)
			// {
				if (cantPulsos > 1)
				{
					switch (dirGiro)
					{
						case 'L':
						motorNro = LEFT_MOTOR;
						vibrateNTimes(LEFT_MOTOR, cantPulsos);
						break;
						case 'R':
						motorNro = RIGHT_MOTOR;
						vibrateNTimes(RIGHT_MOTOR, cantPulsos);
						break;
						case 'F':
						motorNro = FWD_MOTOR;
						vibrateNTimes(FWD_MOTOR, cantPulsos);
						break;
						default:
						break;
					}
					#if DEBUG_PRINTS
					printf("\nHaciendo aviso: motor %d, %d pulsos\n", motorNro, cantPulsos);
					#endif
					// vibrateNTimesLongEnd(cantPulsos);	// Para más de 10 pulsos vibrar largo al final
				} else if (cantPulsos == 1) {
					// vibrateOnceLong(motorNro);					//  Para 1 pulso vibrar largo
				}
			// } else {
				// Si se quiere avisar de la distancia final, vibran N veces los 3 motores
				// for (uint8_t i = 0; i < cantPulsos; i++)
				// {
				// 	MotorOn(LEFT_MOTOR);
				// 	MotorOn(RIGHT_MOTOR);
				// 	MotorOn(FWD_MOTOR);
				// 	ulTaskNotifyTake(pdTRUE, durPulso / portTICK_PERIOD_MS);
				// 	MotorOff(LEFT_MOTOR);
				// 	MotorOff(RIGHT_MOTOR);
				// 	MotorOff(FWD_MOTOR);
				// 	ulTaskNotifyTake(pdTRUE, durPausa / portTICK_PERIOD_MS);
				// }
			// }
		}
		// vTaskDelay(intervaloAvisos_MS / portTICK_PERIOD_MS);
	}
}

/*
 * @brief Función que se llama cuando se presiona el switch 1.
 *
 * @param param Parámetro de la función (no se utiliza en este caso).
 * @note Esta función tiene el efecto de activar o desactivar los avisos de vibración.
*/
void ifSwitch1Pressed(void *param)
{
	if (boolHacerAviso)
	{
		boolHacerAviso = false;
		MotorOff(LEFT_MOTOR);
		MotorOff(RIGHT_MOTOR);
		MotorOff(FWD_MOTOR);
	} else {
		boolHacerAviso = true;
		vTaskNotifyGiveFromISR(hacerAvisoTask_handle, pdFALSE);
	}
}

/*
 * @brief Función que se llama cuando se presiona el switch 2.
 *
 * @param param Parámetro de la función (no se utiliza en este caso).
 * @note Esta función alterna el estado de la variable boolAvisarDistFinal, que indica si se debe avisar
 *       de la distancia final (si es = true) o la distancia al próximo giro (si es = false).
*/
void ifSwitch2Pressed(void *param)
{
	if (!boolAvisarDistFinal)
	{
		boolAvisarDistFinal = true;
	} else {
		boolAvisarDistFinal = false;
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	neopixel_color_t color = NeoPixelRgb2Color(0, 0, 0);
	NeoPixelInit(BUILT_IN_RGB_LED_PIN, BUILT_IN_RGB_LED_LENGTH, &color);
	NeoPixelBrightness(100);
	// LedsInit(); // Temporalmente hasta implementar motores de vibración

	ble_config_t ble_configuration = {
		"ESP32_Feli",
		read_data};
	BleInit(&ble_configuration);

	timer_config_t bleStatusCheckTimer = {
		.timer = TIMER_A,
		.period = 300 * 1000, // 200 ms in microseconds
		.func_p = bleStatusCheck,
		.param_p = NULL
	};
	TimerInit(&bleStatusCheckTimer);

	timer_config_t hacerAvisoTimer = {
		.timer = TIMER_B,
		.period = intervaloAvisos_MS * 1000, // Convertir a microsegundos
		.func_p = hacerAviso,
		.param_p = NULL
	};
	TimerInit(&hacerAvisoTimer);

	// Inicializar motores de vibración
	motor_config_t motorL_Config = {
		.motor_out = LEFT_MOTOR,
		.gpio = GPIO_11,
		.duty_cycle = instensidad,
	};
	MotorInit(&motorL_Config);
		motor_config_t motorF_Config = {
		.motor_out = FWD_MOTOR,
		.gpio = GPIO_10,
		.duty_cycle = instensidad,
	};
	MotorInit(&motorF_Config);
	motor_config_t motorR_Config = {
		.motor_out = RIGHT_MOTOR,
		.gpio = GPIO_5,
		.duty_cycle = instensidad,
	};
	MotorInit(&motorR_Config);
	setMotorPulseDurationMS(durPulso);
	setMotorPauseDurationMS(durPausa);
	distancia = distGiro;
	
	// SwitchActivInt(SWITCH_1, ifSwitch1Pressed, NULL);
	SwitchActivInt(SWITCH_2, ifSwitch2Pressed, NULL);

	xTaskCreate(bleStatusCheckTask, "BleStatusCheckTask", 1024, NULL, 5, &bleStatusCheckTask_handle);
	xTaskCreate(hacerAvisoTask, "HacerAvisoTask", 16384, NULL, 4, &hacerAvisoTask_handle);
	
	TimerStart(bleStatusCheckTimer.timer);
	// TimerStart(hacerAvisoTimer.timer);
}
/*==================[end of file]============================================*/