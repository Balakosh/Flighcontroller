/*
 * sensor.c
 *
 *  Created on: Apr 2, 2022
 *      Author: Akeman
 */

#include <ti/sysbios/knl/Task.h>

#include <ti/drivers/I2C.h>

#include "i2c/i2c.h"

static I2C_Handle MPU6050i2cHandle;

static const uint8_t MPU6050i2cAddress = 0x68;

static uint8_t readRegister(void)
{
    uint8_t txBuffer[1];
    uint8_t rxBuffer[1];
    I2C_Transaction i2cTransaction;

    i2cTransaction.slaveAddress = MPU6050i2cAddress;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = sizeof(txBuffer);
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = sizeof(rxBuffer);

    if (MPU6050i2cHandle != NULL)
    {
        txBuffer[0] = 0x75;

        if (I2C_transfer(MPU6050i2cHandle, &i2cTransaction))
        {
        }
    }

    return rxBuffer[0];
}

void sensorTaskFxn(void)
{
    I2C_Params i2cParams;

    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2cParams.transferMode = I2C_MODE_BLOCKING;

    MPU6050i2cHandle = I2C_open(MPU6050_I2C0, &i2cParams);

    while (1)
    {
        Task_sleep(1000);
    }
}
