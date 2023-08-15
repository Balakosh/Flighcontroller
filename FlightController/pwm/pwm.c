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

static uint32_t clock;
static const uint32_t pwmFrequencyInHertz = 100;

void setPWM1Percent(const uint32_t percent)
{
    if (percent <= 100)
    {
        setPWM1(percent + 500);
    }
}

void setPWM2Percent(const uint32_t percent)
{
    if (percent <= 100)
    {
        setPWM2(percent + 500);
    }
}

void setPWM1(const uint32_t perMil)
{
    static uint32_t lastPerMil;

    if (perMil <= 1000)
    {
        if (perMil != 0.0f)
        {
            if (lastPerMil == 0)
            {
                const unsigned int timerLoad = clock / pwmFrequencyInHertz;
                const uint32_t dutyCycle = timerLoad * ((1000.0f - perMil) / 1000.0f);
                const uint32_t prescaler = timerLoad >> 16;

                TimerPrescaleSet(TIMER3_BASE, TIMER_B, prescaler);

                GPIOPinTypeTimer(GPIO_PORTM_BASE, GPIO_PIN_3);

                TimerLoadSet(TIMER3_BASE, TIMER_B, timerLoad);

                TimerPrescaleMatchSet(TIMER3_BASE, TIMER_B, (dutyCycle >> 16));
                TimerMatchSet(TIMER3_BASE, TIMER_B, (dutyCycle & 0xffff));

                TimerEnable(TIMER3_BASE, TIMER_B);
            }
            else
            {
                const unsigned int timerLoad = clock / pwmFrequencyInHertz;
                const uint32_t dutyCycle = timerLoad * ((1000.0f - perMil) / 1000.0f);

                TimerPrescaleMatchSet(TIMER3_BASE, TIMER_B, (dutyCycle >> 16));
                TimerMatchSet(TIMER3_BASE, TIMER_B, (dutyCycle & 0xffff));
            }

        }
        else
        {
            TimerDisable(TIMER3_BASE, TIMER_B);
            GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_3);
            GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_3, 0);
        }

        lastPerMil = perMil;
    }
}

void setPWM2(const uint32_t perMil)
{
    static uint32_t lastPerMil;

    if (perMil <= 1000)
    {
        if (perMil != 0.0f)
        {
            if (lastPerMil == 0)
            {
                const unsigned int timerLoad = clock / pwmFrequencyInHertz;
                const uint32_t dutyCycle = timerLoad * ((1000.0f - perMil) / 1000.0f);
                const uint32_t prescaler = timerLoad >> 16;

                TimerPrescaleSet(TIMER5_BASE, TIMER_B, prescaler);

                GPIOPinTypeTimer(GPIO_PORTM_BASE, GPIO_PIN_7);

                TimerLoadSet(TIMER5_BASE, TIMER_B, timerLoad);

                TimerPrescaleMatchSet(TIMER5_BASE, TIMER_B, (dutyCycle >> 16));
                TimerMatchSet(TIMER5_BASE, TIMER_B, (dutyCycle & 0xffff));

                TimerEnable(TIMER5_BASE, TIMER_B);
            }
            else
            {
                const unsigned int timerLoad = clock / pwmFrequencyInHertz;
                const uint32_t dutyCycle = timerLoad * ((1000.0f - perMil) / 1000.0f);

                TimerPrescaleMatchSet(TIMER5_BASE, TIMER_B, (dutyCycle >> 16));
                TimerMatchSet(TIMER5_BASE, TIMER_B, (dutyCycle & 0xffff));
            }

        }
        else
        {
            TimerDisable(TIMER5_BASE, TIMER_B);
            GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_7);
            GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_7, 0);
        }

        lastPerMil = perMil;
    }
}

static void initPWM1(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);

    GPIOPinConfigure(GPIO_PM3_T3CCP1);
    GPIOPinTypeTimer(GPIO_PORTM_BASE, GPIO_PIN_3);

    TimerConfigure(TIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM);
    TimerClockSourceSet(TIMER3_BASE, TIMER_CLOCK_SYSTEM);
}

static void initPWM2(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);

    GPIOPinConfigure(GPIO_PM7_T5CCP1);
    GPIOPinTypeTimer(GPIO_PORTM_BASE, GPIO_PIN_7);

    TimerConfigure(TIMER5_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM);
    TimerClockSourceSet(TIMER5_BASE, TIMER_CLOCK_SYSTEM);
}

void initPWM(void)
{
    xdc_runtime_Types_FreqHz freq;
    BIOS_getCpuFreq((xdc_runtime_Types_FreqHz*)&freq);

    clock = freq.lo;

    initPWM1();
    initPWM2();

    setPWM1Percent(0);
}
