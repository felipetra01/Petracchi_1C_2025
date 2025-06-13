/**
 * @file vibration_motor.c
 * @author Felipe M.Petracchi (felipe.petracchi@ingenieria.uner.edu.ar)
 * @brief 
 * @version 0.1
 * @date 2025-06-10
 * 
 * @copyright Copyright (c) 2025
 * 
 */

/*==================[inclusions]=============================================*/
#include "vibration_motor.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
uint8_t MotorInit(motor_config_t *motor) {
    switch(motor->pwm_out){
        case PWM_0:
        PWMInit(PWM_0, motor->gpio, MOTOR_FREQ);
        PWMSetDutyCycle(PWM_0, motor->duty_cycle);
        break;
        case PWM_1:
        PWMInit(PWM_1, motor->gpio, MOTOR_FREQ);
        PWMSetDutyCycle(PWM_1, motor->duty_cycle);
        break;
        case PWM_2:
        PWMInit(PWM_2, motor->gpio, MOTOR_FREQ);
        PWMSetDutyCycle(PWM_2, motor->duty_cycle);
        break;
        case PWM_3:
        PWMInit(PWM_3, motor->gpio, MOTOR_FREQ);
        PWMSetDutyCycle(PWM_3, motor->duty_cycle);
        break;
	}
    // Set the initial state of the motor to off
    PWMOff(motor->pwm_out);
    // timer_config_t timerPulsos = {
    //     .timer = motor->pwm_out,
    //     .period = (PULSE_WIDTH_MS + PAUSE_MS) * 1000,
    //     .func_p = hacerPulso,
    //     .param_p = NULL
    // };
	return 0;
}

void MotorOn(motor_out_t motor) {
    switch(motor){
        case MOTOR_0:
            PWMOn(PWM_0);
            break;
        case MOTOR_1:
            PWMOn(PWM_1);
            break;
        case MOTOR_2:
            PWMOn(PWM_2);
            break;
        case MOTOR_3:
            PWMOn(PWM_3);
            break;
    }
}

void MotorOff(motor_out_t motor) {
    switch(motor){
        case MOTOR_0:
            PWMOff(PWM_0);
            break;
        case MOTOR_1:
            PWMOff(PWM_1);
            break;
        case MOTOR_2:
            PWMOff(PWM_2);
            break;
        case MOTOR_3:
            PWMOff(PWM_3);
            break;
    }
}

void vibrateNTimes(motor_out_t motor, uint8_t N) {
    for (uint8_t i = 0; i < N; i++) {
        PWMOn(motor);
        vTaskDelay(PULSE_WIDTH_MS / portTICK_PERIOD_MS);
        PWMOff(motor);
        vTaskDelay(PAUSE_MS / portTICK_PERIOD_MS);
    }
}
/*==================[external functions definition]==========================*/
