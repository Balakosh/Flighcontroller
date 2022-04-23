/*
 * mpu6050.c
 *
 *  Created on: Apr 2, 2022
 *      Author: Akeman
 */

#include <stdint.h>
#include <string.h>

#include <ti/drivers/I2C.h>

#include <ti/sysbios/knl/Semaphore.h>

#include "i2c/i2c.h"
#include "mpu6050.h"
#include "utils/twoComplement.h"

static I2C_Handle MPU6050i2cHandle;

static const uint8_t MPU6050i2cAddress = 0x68;

Semaphore_Struct mpu6050InterruptSemaphoreStruct;
Semaphore_Handle mpu6050InterruptSemaphoreHandle;

static float accelXinG;
static float accelYinG;
static float accelZinG;

static float omgXinDegPerSec;
static float omgYinDegPerSec;
static float omgZinDegPerSec;

static float temperatureInCelsius;

static uint8_t readRegister(const uint8_t registerNumber)
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
        txBuffer[0] = registerNumber;

        if (I2C_transfer(MPU6050i2cHandle, &i2cTransaction))
        {
            return rxBuffer[0];
        }
    }

    return rxBuffer[0];
}

static bool readRegisterBurst(const uint8_t registerNumber, const uint8_t* rxBuffer, const uint32_t size)
{
    I2C_Transaction i2cTransaction;

    i2cTransaction.slaveAddress = MPU6050i2cAddress;
    i2cTransaction.writeBuf = (void*)&registerNumber;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = (void*)rxBuffer;
    i2cTransaction.readCount = size;

    if (MPU6050i2cHandle != NULL)
    {
        if (I2C_transfer(MPU6050i2cHandle, &i2cTransaction))
        {
            return true;
        }
    }

    return false;
}

static bool writeRegister(const uint8_t registerNumber, const uint8_t value)
{
    uint8_t txBuffer[2];
    I2C_Transaction i2cTransaction;

    i2cTransaction.slaveAddress = MPU6050i2cAddress;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = sizeof(txBuffer);
    i2cTransaction.readCount = 0;

    if (MPU6050i2cHandle != NULL)
    {
        txBuffer[0] = registerNumber;
        txBuffer[1] = value;

        if (I2C_transfer(MPU6050i2cHandle, &i2cTransaction))
        {
            return true;
        }
    }

    return false;
}

static void openI2C(void)
{
    I2C_Params i2cParams;

    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2cParams.transferMode = I2C_MODE_BLOCKING;

    MPU6050i2cHandle = I2C_open(MPU6050_I2C0, &i2cParams);
}

static void resetMPU6050(void)
{
    writeRegister(MPU6050_SIGNAL_PATH_RESET, MPU6050_RESET_TEMP | MPU6050_RESET_ACC | MPU6050_RESET_GYRO);
}

static void resetMPU6050AndClearRegisters(void)
{
    writeRegister(MPU6050_USER_CTRL, MPU6050_RESET | MPU6050_FIFO_RESET | MPU6050_FIFO_ENABLE);
}

static void enableInterruptsMPU6050(void)
{
    writeRegister(MPU6050_INT_ENABLE, MPU6050_DATA_READY_INT_EN);
}

static void configurePowerManagementMPU6050(void)
{
    writeRegister(MPU6050_PWR_MGMT_1, 0);
    writeRegister(MPU6050_PWR_MGMT_2, 0);
}

static void configureSampleRateDividerMPU6050(void)
{
    writeRegister(MPU6050_SAMPLE_RATE_DIVIDER, 0);
}

static void configureMPU6050(void)
{
    writeRegister(MPU6050_CONFIG, 1);
}

static void configureFIFOMPU6050(void)
{
    writeRegister(MPU6050_FIFO_EN, MPU6050_FIFO_ACC | MPU6050_FIFO_GYRO | MPU6050_FIFO_TEMP);
}

void initMPU6050(void)
{
    openI2C();

    configurePowerManagementMPU6050();
    resetMPU6050AndClearRegisters();
    enableInterruptsMPU6050();
    configureMPU6050();
    configureSampleRateDividerMPU6050();
    configureFIFOMPU6050();

    const uint8_t whoami = readRegister(MPU6050_WHO_AM_I);
    const uint8_t pwrManagement1 = readRegister(MPU6050_PWR_MGMT_1);
    const uint8_t pwrManagement2 = readRegister(MPU6050_PWR_MGMT_2);

    writeRegister(MPU6050_ACCEL_CONFIG, MPU6050_ACC_CONFIG_2G);
    const uint8_t accelConfig = readRegister(MPU6050_ACCEL_CONFIG);

    writeRegister(MPU6050_GYRO_CONFIG, MPU6050_GYRO_CONFIG_250_DEG_PER_S);
    const uint8_t gyroConfig = readRegister(MPU6050_GYRO_CONFIG);
}

void readAccelerometer(void)
{
    uint8_t accBuffer[6];

    if (readRegisterBurst(MPU6050_ACCEL_XOUT_H, accBuffer, sizeof(accBuffer)))
    {
        accelXinG = ConvertTwosComplementShortToInteger((accBuffer[0] << 8) | accBuffer[1]) / 16384.0f;
        accelYinG = ConvertTwosComplementShortToInteger((accBuffer[2] << 8) | accBuffer[3]) / 16384.0f;
        accelZinG = ConvertTwosComplementShortToInteger((accBuffer[4] << 8) | accBuffer[5]) / 16384.0f;
    }
}

void readGyroscope(void)
{
    uint8_t gyroBuffer[6];

    if (readRegisterBurst(MPU6050_GYRO_XOUT_H, gyroBuffer, sizeof(gyroBuffer)))
    {
        omgXinDegPerSec = ConvertTwosComplementShortToInteger((gyroBuffer[0] << 8) | gyroBuffer[1]) / 131.0f;
        omgYinDegPerSec = ConvertTwosComplementShortToInteger((gyroBuffer[2] << 8) | gyroBuffer[3]) / 131.0f;
        omgZinDegPerSec = ConvertTwosComplementShortToInteger((gyroBuffer[4] << 8) | gyroBuffer[5]) / 131.0f;
    }
}

void readTemperature(void)
{
    uint8_t tempBuffer[2];

    if (readRegisterBurst(MPU6050_TEMP_OUT_H, tempBuffer, sizeof(tempBuffer)))
    {
        temperatureInCelsius = ((int16_t)((tempBuffer[0] << 8) | tempBuffer[1]) / 340.0f) + 36.53f;
    }
}

uint16_t getFifoCount(void)
{
    uint8_t buffer[2];

    if (readRegisterBurst(MPU6050_FIFO_COUNTH, buffer, sizeof(buffer)))
    {
        return (uint16_t)((buffer[0] << 8) | buffer[1]);
    }

    return 0;
}

static uint8_t getFifoValue(void)
{
    uint8_t buffer = 0;

    if (readRegisterBurst(MPU6050_FIFO_R_W, &buffer, sizeof(buffer)))
    {
        return buffer;
    }

    return 0;
}

uint8_t getInterruptStatus(void)
{
    uint8_t buffer = 0;

    if (readRegisterBurst(MPU6050_INT_STATUS, &buffer, sizeof(buffer)))
    {
        return buffer;
    }

    return 0;
}

MPU6050_Data getFifoValues(void)
{
    MPU6050_FifoData data;
    MPU6050_Data convertedData;

    memset(&data, 0, sizeof(MPU6050_FifoData));
    memset(&convertedData, 0, sizeof(MPU6050_Data));

    if (readRegisterBurst(MPU6050_FIFO_R_W, (uint8_t*)&data, sizeof(data)))
    {
        convertedData.accelX = ConvertTwosComplementShortToInteger(data.accelX) / 16384.0;
        convertedData.accelY = ConvertTwosComplementShortToInteger(data.accelY) / 16384.0;
        convertedData.accelZ = ConvertTwosComplementShortToInteger(data.accelZ) / 16384.0;

        convertedData.temperature = (data.temperature / 340.0) + 36.53;

        convertedData.gyroX = ConvertTwosComplementShortToInteger(data.gyroX) / 131.0;
        convertedData.gyroY = ConvertTwosComplementShortToInteger(data.gyroY) / 131.0;
        convertedData.gyroZ = ConvertTwosComplementShortToInteger(data.gyroZ) / 131.0;

        return convertedData;
    }

    return convertedData;
}
