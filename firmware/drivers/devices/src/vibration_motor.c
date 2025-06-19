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
uint16_t pulse_ms = PULSE_MS_DEFAULT;
uint16_t pause_ms = PAUSE_MS_DEFAULT;
/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
uint8_t MotorInit(motor_config_t *motor) {
    switch(motor->motor_out){
        case PWM_0:
        PWMInit(PWM_0, motor->gpio, MOTOR_FREQ_DEFAULT);
        PWMSetDutyCycle(PWM_0, motor->duty_cycle);
        break;
        case PWM_1:
        PWMInit(PWM_1, motor->gpio, MOTOR_FREQ_DEFAULT);
        PWMSetDutyCycle(PWM_1, motor->duty_cycle);
        break;
        case PWM_2:
        PWMInit(PWM_2, motor->gpio, MOTOR_FREQ_DEFAULT);
        PWMSetDutyCycle(PWM_2, motor->duty_cycle);
        break;
        case PWM_3:
        PWMInit(PWM_3, motor->gpio, MOTOR_FREQ_DEFAULT);
        PWMSetDutyCycle(PWM_3, motor->duty_cycle);
        break;
	}
    // Set the initial state of the motor to off
    PWMOff(motor->motor_out);
	return 0;
}

void setMotorPulseDurationMS(uint16_t time_ms) {
    if (time_ms > 0) {
        pulse_ms = time_ms;
    }
}

void setMotorPauseDurationMS(uint16_t time_ms) {
    if (time_ms > 0) {
        pause_ms = time_ms;
    }
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

        // DelayMs(pulse_ms);
        // vTaskDelay(pulse_ms / portTICK_PERIOD_MS);
        ulTaskNotifyTake(pdTRUE, pulse_ms / portTICK_PERIOD_MS);

        PWMOff(motor);

        // DelayMs(pause_ms);
        // vTaskDelay(pause_ms / portTICK_PERIOD_MS);
        ulTaskNotifyTake(pdTRUE, pause_ms / portTICK_PERIOD_MS);
    }
    PWMOff(motor);
}
/*==================[external functions definition]==========================*/
