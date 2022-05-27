/*
 * sensor.c
 *
 *  Created on: Apr 2, 2022
 *      Author: Akeman
 */

#include <math.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#include <ti/drivers/I2C.h>
#include <ti/sysbios/gates/GateSwi.h>

#include "i2c/i2c.h"
#include "mpu6050/mpu6050.h"
#include "filter/madgwick/MadgwickAHRS.h"
#include "sensor.h"
#include "eth/tcpServer.h"

static MPU6050_Data data;
static MPU6050_Comstats mpu6050Comstats;
static RollPitchYawInRad eulersRad;

GateSwi_Handle sensorDataSwiGateHandle;
GateSwi_Struct sensorDataSwiGateStruct;

static void calculateEulerAnglesFromQuaternions(void)
{
    eulersRad.roll = atan2(2.0 * q0 * q1 + 2.0 * q2 * q3, q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3);
    eulersRad.pitch = asin(2.0 * q0 * q2 - 2.0 * q1 * q3);
    eulersRad.yaw = 0;//atan2(2.0 * q0 * q3 + 2.0 * q1 * q2, q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3);
}

static void dataReadyInterruptHandler(void)
{
    MPU6050_Data localData;

    const uint16_t fifoCount = getFifoCount();

    if (fifoCount >= 14)
    {
        if (getFifoValues(&localData))
        {
            localData.valid = true;

            MadgwickAHRSupdate(localData.gyroX / 180.0 * M_PI, localData.gyroY / 180.0 * M_PI, localData.gyroZ / 180.0 * M_PI, localData.accelX, localData.accelY, localData.accelZ, 0.0, 0.0, 0.0);
        }
        else
        {
            localData.valid = false;
            mpu6050Comstats.dataInvalidCounter++;
        }

        const IArg gateKey = GateSwi_enter(sensorDataSwiGateHandle);

        calculateEulerAnglesFromQuaternions();
        memcpy(&data, &localData, sizeof(MPU6050_Data));

        if (fifoCount > mpu6050Comstats.fifoCountPeak)
        {
            mpu6050Comstats.fifoCountPeak = fifoCount;
        }

        GateSwi_leave(sensorDataSwiGateHandle, gateKey);
    }
}

static void fifoOverflowInterruptHandler(void)
{
    mpu6050Comstats.overflowCounter++;

    initMPU6050();
}

void sensorTaskFxn(void)
{
    openI2C();
    initMPU6050();

    while (1)
    {
        Semaphore_pend(mpu6050InterruptSemaphoreHandle, BIOS_WAIT_FOREVER);

        const uint8_t interruptStatus = getInterruptStatus();

        if ((interruptStatus & MPU6050_DATA_READY_INT) == MPU6050_DATA_READY_INT)
        {
            dataReadyInterruptHandler();
        }

        if ((interruptStatus & MPU6050_FIFO_OVERFLOW_INT) == MPU6050_FIFO_OVERFLOW_INT)
        {
            fifoOverflowInterruptHandler();
        }
    }
}

MPU6050_Data getMPU6050Data(void)
{
    return data;
}

MPU6050_Comstats getMPU6050Comstats(void)
{
    return mpu6050Comstats;
}

RollPitchYawInRad getEulerAngles(void)
{
    return eulersRad;
}
