/*
 * pwm.h
 *
 *  Created on: Mar 31, 2022
 *      Author: Akeman
 */

#ifndef PWM_PWM_H_
#define PWM_PWM_H_

void initPWM(void);
void setPWM1(uint32_t perMil);
void setPWM2(uint32_t perMil);
void setPWM1Percent(uint32_t percent);
void setPWM2Percent(uint32_t percent);

#endif /* PWM_PWM_H_ */
