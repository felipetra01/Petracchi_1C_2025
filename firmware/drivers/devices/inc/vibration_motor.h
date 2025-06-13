#ifndef VIBRATION_MOTOR_H
#define VIBRATION_MOTOR_H
/** \addtogroup Drivers_Programable Drivers Programable
 ** @{ */
/** \addtogroup Drivers_Devices Drivers devices
 ** @{ */
/** \addtogroup VIBRATION_MOTOR Vibration Motor
 ** @{ */

/** @brief VIBRATION_MOTOR driver
 *
 * @note This driver can handle up to 4 vibration motors. It provides functions to control them with PWM signals.
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
#include <stdio.h>
#include <stdint.h>
#include "sdkconfig.h"
#include "pwm_mcu.h"
#include "gpio_mcu.h"

/*==================[macros]=================================================*/
/*
 * @brief Default PWM frequency for the vibration motors
 *
 * The default frequency is set to 1000 Hz, which is suitable for most vibration motors.
 * This can be adjusted based on the specific motor requirements.
*/
#define MOTOR_FREQ 	   1000     // Default PWM frequency in Hz
#define PULSE_WIDTH_MS 50       // Pulse width in milliseconds
#define PAUSE_MS       50
#define PERCENT   	   100.0
/*==================[typedef]================================================*/
typedef enum motor_out {
	MOTOR_0 = PWM_0,    /**< Vibration motor 1 */
    MOTOR_1 = PWM_1,    /**< Vibration motor 2 */
    MOTOR_2 = PWM_2,    /**< Vibration motor 3 */
    MOTOR_3 = PWM_3     /**< Vibration motor 4 */
} motor_out_t;

typedef struct {
    motor_out_t pwm_out; /**< Motor to control */
    gpio_t gpio;        /**< GPIO pin for the motor */
    uint8_t duty_cycle; /**< Duty cycle in percentage (0-100) */
} motor_config_t;
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions declaration]=========================*/

uint8_t MotorInit(motor_config_t *motor);

void MotorOn(motor_out_t motor);

void MotorOff(motor_out_t motor);

void vibrateNTimes(motor_out_t motor, uint8_t N);

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
#endif /* PWM_MCU_H_ */

/*==================[end of file]============================================*/