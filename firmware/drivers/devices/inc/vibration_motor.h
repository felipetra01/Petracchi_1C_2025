#ifndef VIBRATION_MOTOR_H
#define VIBRATION_MOTOR_H
/** \addtogroup Drivers_Programable Drivers Programable
 ** @{ */
/** \addtogroup Drivers_Microcontroller Drivers microcontroller
 ** @{ */
/** \addtogroup VIBRATION_MOTOR VIBRATION_MOTOR
 ** @{ */

/** @brief VIBRATION_MOTOR driver
 *
 * This driver provide functions to control a vibration motor with PWM signals
 *
 * @note
 *
 * @author Felipe M. Petracchi
 * 
 * @section changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 06/06/2025 | Document creation		                         |
 *
 */

/*==================[inclusions]=============================================*/
#include <stdint.h>
#include "sdkconfig.h"
#include "pwm_mcu.h"
#include "gpio_mcu.h"
/*==================[macros]=================================================*/
#define MOTOR_FREQ 	1000		/**< PWM frequency */
#define PERIOD_MS   20.0
#define PULSEW_MS   1.0
#define PERCENT   	100.0
/*==================[typedef]================================================*/
typedef enum motor_out {
	MOTOR_0 = PWM_0, /**< Vibration motor 1 */
    MOTOR_1 = PWM_1, /**< Vibration motor 2 */
    MOTOR_2 = PWM_2,  /**< Vibration motor 3 */
    MOTOR_3 = PWM_3  /**< Vibration motor 4 */
} motor_out_t;

typedef struct {
    motor_out_t pwm_out; /**< Motor to control */
    gpio_t gpio;        /**< GPIO pin for the motor */
    uint8_t duty_cycle; /**< Duty cycle in percentage (0-100) */
    uint8_t freq;   /**< Frequency in Hz */
} motor_config_t;
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions declaration]=========================*/

uint8_t MotorInit(motor_out_t motor, gpio_t gpio);

void MotorOn(motor_out_t motor);

void MotorOff(motor_out_t motor);

void vibrateNTimes(motor_out_t motor, uint8_t N);

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
#endif /* PWM_MCU_H_ */

/*==================[end of file]============================================*/