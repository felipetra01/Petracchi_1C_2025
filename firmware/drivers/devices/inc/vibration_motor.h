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
#include <delay_mcu.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*==================[macros]=================================================*/
/*
 * @brief Default PWM frequency for the vibration motors
 *
 * The default frequency is set to 1000 Hz, which is suitable for most vibration motors.
 * This can be adjusted based on the specific motor requirements.
*/
#define MOTOR_FREQ_DEFAULT  2000    // Default PWM frequency in Hz
#define PULSE_MS_DEFAULT    400     // Default pulse duration (width) in milliseconds
#define PAUSE_MS_DEFAULT    400     // Default pause duration (width) in 
#define LONG_PULSE_MS_DEFAULT  1000    // Default long pulse duration (width) in milliseconds
/*==================[typedef]================================================*/
typedef enum motor_out {
	MOTOR_0 = PWM_0,    /**< Vibration motor 1 */
    MOTOR_1 = PWM_1,    /**< Vibration motor 2 */
    MOTOR_2 = PWM_2,    /**< Vibration motor 3 */
    MOTOR_3 = PWM_3     /**< Vibration motor 4 */
} motor_out_t;

typedef struct {
    motor_out_t motor_out; /**< Motor to control */
    gpio_t gpio;        /**< GPIO pin for the motor */
    uint8_t duty_cycle; /**< Duty cycle in percentage (0-100) */
} motor_config_t;
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief This function initializes the specified motor with the given configuration.
 *
 * @param motor Pointer to the motor configuration structure.
 * @return uint8_t Returns 0 on success, non-zero on failure.
 */
uint8_t MotorInit(motor_config_t *motor);

/**
 * @brief Sets the duration of the motor pulse in milliseconds.
 * 
 * @param time_ms Duration in milliseconds.
 */
void setMotorPulseDurationMS(uint16_t time_ms);

/**
 * @brief Sets the pause duration between motor pulses in milliseconds.
 *
 * @param time_ms Duration in milliseconds.
 */
void setMotorPauseDurationMS(uint16_t time_ms);

/*
 * @brief Turns on the specified motor.
 *
 * @param motor The motor to turn on.
 */
void MotorOn(motor_out_t motor);

/**
 * @brief Turns off the specified motor.
 *
 * @param motor The motor to turn off.
 */
void MotorOff(motor_out_t motor);

/**
 * @brief Vibrates the specified motor for a single pulse.
 *
 * @param motor The motor to vibrate.
 */
void vibrateNTimes(motor_out_t motor, uint8_t N);

/**
 * @brief Vibrates the specified motor once with one long pulse.
 *
 * @param motor The motor to vibrate.
 */
void vibrateOnceLong(motor_out_t motor);

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
#endif /* VIBRATION_MOTOR_H_ */

/*==================[end of file]============================================*/