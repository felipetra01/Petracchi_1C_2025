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
 * | 	PIN_X	 	| 	GPIO_X		|
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

#include "dht11.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
void initComponentes(void) {
	LedsInit();
	// DHT11Init(GPIO); // Si el sensor lo requiere. 
}
/*==================[external functions definition]==========================*/
void app_main(void){
	initComponentes();

}
/*==================[end of file]============================================*/