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

#include "i2c/i2c.h"
#include "mpu6050/mpu6050.h"
#include "filter/madgwick/MadgwickAHRS.h"
#include "sensor.h"

static MPU6050_Data data;
static RollPitchYawInRad eulersRad;

static void calculateEulerAnglesFromQuaternions(void)
{
    eulersRad.roll = atan2(2.0 * q0 * q1 + 2.0 * q2 * q3, q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3);
    eulersRad.pitch = asin(2.0 * q0 * q2 - 2.0 * q1 * q3);
    eulersRad.yaw = atan2(2.0 * q0 * q3 + 2.0 * q1 * q2, q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3);
}

void sensorTaskFxn(void)
{
    initMPU6050();

    while (1)
    {
        Semaphore_pend(mpu6050InterruptSemaphoreHandle, BIOS_WAIT_FOREVER);

        const uint8_t interruptStatus = getInterruptStatus();

        const uint16_t fifoCount = getFifoCount();

        if (fifoCount >= 1)
        {
            if (getFifoValues(&data))
            {
                data.valid = true;

                MadgwickAHRSupdate(data.gyroX / 180.0 * M_PI, data.gyroY / 180.0 * M_PI, data.gyroZ / 180.0 * M_PI, data.accelX, data.accelY, data.accelZ, 0.0, 0.0, 0.0);
                calculateEulerAnglesFromQuaternions();
            }
            else
            {
                data.valid = false;
            }


        }
    }
}

MPU6050_Data getMPU6050Data(void)
{
    return data;
}

RollPitchYawInRad getEulerAngles(void)
{
    return eulersRad;
}
