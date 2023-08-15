/*
 * pwm.c
 *
 *  Created on: Mar 29, 2022
 *      Author: Akeman
 */

#include <stdbool.h>
#include <stdint.h>

#include <ti/sysbios/BIOS.h>

#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_gpio.h>

#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>

#include "pwm/pwm.h"

static const uint32_t pwmFrequencyInHertz = 100;
static uint32_t timerLoad;
static pwmConfig pwmConfigs[2] =
{
     { SYSCTL_PERIPH_TIMER3, GPIO_PM3_T3CCP1, GPIO_PORTM_BASE, GPIO_PIN_3, TIMER3_BASE, TIMER_B },
     { SYSCTL_PERIPH_TIMER5, GPIO_PM7_T5CCP1, GPIO_PORTM_BASE, GPIO_PIN_7, TIMER5_BASE, TIMER_B },
};

void setPWMPercent(const uint32_t percent, const uint8_t pwmNumber)
{
    if (percent <= 100)
    {
        setPWM(percent + 500, pwmNumber);
    }
}

void setPWM(const uint32_t perMil, const uint8_t pwmNumber)
{
    if (perMil <= 1000)
    {
        if (perMil != 0.0f)
        {
            if (pwmConfigs[pwmNumber].lastPerMil == 0)
            {
                const uint32_t dutyCycle = timerLoad * ((1000.0f - perMil) / 1000.0f);
                const uint32_t prescaler = timerLoad >> 16;

                TimerPrescaleSet(pwmConfigs[pwmNumber].timerBase, pwmConfigs[pwmNumber].timer, prescaler);

                GPIOPinTypeTimer(pwmConfigs[pwmNumber].port, pwmConfigs[pwmNumber].pin);

                TimerLoadSet(pwmConfigs[pwmNumber].timerBase, pwmConfigs[pwmNumber].timer, timerLoad);

                TimerPrescaleMatchSet(pwmConfigs[pwmNumber].timerBase, pwmConfigs[pwmNumber].timer, (dutyCycle >> 16));
                TimerMatchSet(pwmConfigs[pwmNumber].timerBase, pwmConfigs[pwmNumber].timer, (dutyCycle & 0xffff));

                TimerEnable(pwmConfigs[pwmNumber].timerBase, pwmConfigs[pwmNumber].timer);
            }
            else
            {
                const uint32_t dutyCycle = timerLoad * ((1000.0f - perMil) / 1000.0f);

                TimerPrescaleMatchSet(pwmConfigs[pwmNumber].timerBase, pwmConfigs[pwmNumber].timer, (dutyCycle >> 16));
                TimerMatchSet(pwmConfigs[pwmNumber].timerBase, pwmConfigs[pwmNumber].timer, (dutyCycle & 0xffff));
            }

        }
        else
        {
            TimerDisable(pwmConfigs[pwmNumber].timerBase, TIMER_B);
            GPIOPinTypeGPIOOutput(pwmConfigs[pwmNumber].port, pwmConfigs[pwmNumber].pin);
            GPIOPinWrite(pwmConfigs[pwmNumber].port, pwmConfigs[pwmNumber].pin, 0);
        }

        pwmConfigs[pwmNumber].lastPerMil = perMil;
    }
}

void initPWM(void)
{
    xdc_runtime_Types_FreqHz freq;
    BIOS_getCpuFreq((xdc_runtime_Types_FreqHz*)&freq);

    timerLoad = freq.lo / pwmFrequencyInHertz;

    for (int i = 0; i < (sizeof(pwmConfigs) / sizeof(pwmConfig)); i++)
    {
        SysCtlPeripheralEnable(pwmConfigs[i].peripheral);

        GPIOPinConfigure(pwmConfigs[i].pinMux);
        GPIOPinTypeTimer(pwmConfigs[i].port, pwmConfigs[i].pin);

        TimerConfigure(pwmConfigs[i].timerBase, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM);
        TimerClockSourceSet(pwmConfigs[i].timerBase, TIMER_CLOCK_SYSTEM);

        setPWMPercent(0, i);
    }
}
