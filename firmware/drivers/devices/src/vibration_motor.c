/**
 * @file pwm_mcu.c
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 * @brief 
 * @version 0.1
 * @date 2024-01-23
 * 
 * @copyright Copyright (c) 2023
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
uint8_t MotorInit(motor_out_t motor, gpio_t gpio) {
	switch(motor){
		case SERVO_0:
			PWMInit(motor.pwm_out, gpio, MOTOR_FREQ);
            PWMSetDutyCycle(motor.pwm_out, motor.duty_cycle);
			break;
		case SERVO_1:
			PWMInit(PWM_1, gpio, MOTOR_FREQ);
			break;
		case SERVO_2:
			PWMInit(PWM_2, gpio, MOTOR_FREQ);
			break;
		case SERVO_3:
			PWMInit(PWM_3, gpio, MOTOR_FREQ);
			break;
	}
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

}
/*==================[external functions definition]==========================*/
