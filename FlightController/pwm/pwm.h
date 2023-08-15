/*
 * pwm.h
 *
 *  Created on: Mar 31, 2022
 *      Author: Akeman
 */

#ifndef PWM_PWM_H_
#define PWM_PWM_H_

typedef struct
{
    uint32_t peripheral;
    uint32_t pinMux;
    uint32_t port;
    uint8_t pin;
    uint32_t timerBase;
    uint32_t timer;
    uint32_t lastPerMil;
} pwmConfig;

void initPWM(void);
void setPWM(uint32_t perMil, uint8_t pwmNumber);
void setPWMPercent(uint32_t percent, uint8_t pwmNumber);

#endif /* PWM_PWM_H_ */
