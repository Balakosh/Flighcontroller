/*
 * esc.c
 *
 *  Created on: Apr 2, 2022
 *      Author: Akeman
 */

#include <ti/sysbios/knl/Clock.h>

#include "pwm/pwm.h"

Clock_Struct escCalibrationClockStruct;
Clock_Handle escCalibrationClockHandle;

static uint32_t pwmDutyCycleInPercent = 100;

void escCalibrationClockFxn(void)
{
    pwmDutyCycleInPercent -= 1;

    setPWMPercent(pwmDutyCycleInPercent, 0);
    setPWMPercent(pwmDutyCycleInPercent, 1);

    if (pwmDutyCycleInPercent == 10)
    {
        Clock_stop(escCalibrationClockHandle);
    }
}

void calibrateESC(void)
{
    pwmDutyCycleInPercent = 90;
    setPWMPercent(pwmDutyCycleInPercent, 0);
    setPWMPercent(pwmDutyCycleInPercent, 1);
    Clock_start(escCalibrationClockHandle);
}
