/*
 * i2c.c
 *
 *  Created on: Apr 2, 2022
 *      Author: Akeman
 */

#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>

#include <ti/drivers/I2C.h>
#include <ti/drivers/i2c/I2CTiva.h>

#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <driverlib/gpio.h>

#include "i2c/i2c.h"

I2CTiva_Object i2cTivaObjects[i2cCount];

const I2CTiva_HWAttrs i2cTivaHWAttrs[i2cCount] =
{
    {
        .baseAddr = I2C0_BASE,
        .intNum = INT_I2C0,
        .intPriority = (~0)
    }
};

const I2C_Config I2C_config[] =
{
    {
        .fxnTablePtr = &I2CTiva_fxnTable,
        .object = &i2cTivaObjects[0],
        .hwAttrs = &i2cTivaHWAttrs[0]
    },
    {NULL, NULL, NULL}
};

void initI2C(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);

    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    I2C_init();
}
