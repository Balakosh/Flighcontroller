/*
 * gpio.c
 *
 *  Created on: Mar 27, 2022
 *      Author: Akeman
 */

#include <stdint.h>
#include <stdbool.h>

#include <xdc/std.h>

#include <ti/drivers/GPIO.h>
#include <ti/drivers/gpio/GPIOTiva.h>

#include "gpio/gpio.h"
#include "sensors/mpu6050/mpu6050.h"

GPIO_PinConfig gpioPinConfigs[] =
{
    /* Input pins */
    GPIOTiva_PJ_0 | GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_RISING,
    GPIOTiva_PJ_1 | GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_RISING,
    GPIOTiva_PC_7 | GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_RISING,

    /* Output pins */
    /* EK_TM4C1294XL_USR_D1 */
    GPIOTiva_PN_1 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,
    /* EK_TM4C1294XL_USR_D2 */
    GPIOTiva_PN_0 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW
};

_Static_assert ((sizeof(gpioPinConfigs) != EK_TM4C1294XL_GPIOCOUNT), "sizeof(gpioPinConfigs) != EK_TM4C1294XL_GPIOCOUNT");

/*
 * Array of callback function pointers
 * NOTE: The order of the pin configurations must coincide with what was
 *       defined in EK_TM4C1294XL.h
 * NOTE: Pins not used for interrupts can be omitted from callbacks array to
 *       reduce memory usage (if placed at end of gpioPinConfigs array).
 */
GPIO_CallbackFxn gpioCallbackFunctions[] =
{
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL
};

/* The device-specific GPIO_config structure */
const GPIOTiva_Config GPIOTiva_config =
{
    .pinConfigs = (GPIO_PinConfig*)gpioPinConfigs,
    .callbacks = (GPIO_CallbackFxn*)gpioCallbackFunctions,
    .numberOfPinConfigs = sizeof(gpioPinConfigs) / sizeof(GPIO_PinConfig),
    .numberOfCallbacks = sizeof(gpioCallbackFunctions) / sizeof(GPIO_CallbackFxn),
    .intPriority = (~0)
};

void mpu6050Hwi(unsigned int pinState)
{
    static int counter;

    counter++;

    Semaphore_post(mpu6050InterruptSemaphoreHandle);
}

void createGPIOCallbacks(void)
{
    GPIO_setCallback(MPU6050_INTERRUPT, mpu6050Hwi);
    GPIO_enableInt(MPU6050_INTERRUPT);
}

void toggleLEDD1(void)
{
    GPIO_toggle(EK_TM4C1294XL_D1);
}

void initGPIO(void)
{
    GPIO_init();

    createGPIOCallbacks();
}
