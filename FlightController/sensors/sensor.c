/*
 * sensor.c
 *
 *  Created on: Apr 2, 2022
 *      Author: Akeman
 */

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#include <ti/drivers/I2C.h>

#include "i2c/i2c.h"
#include "mpu6050/mpu6050.h"
#include "filter/madgwick/MadgwickAHRS.h"

static MPU6050_Data data;

void sensorTaskFxn(void)
{
    initMPU6050();

    while (1)
    {
        Semaphore_pend(mpu6050InterruptSemaphoreHandle, BIOS_WAIT_FOREVER);

        const uint8_t interruptStatus = getInterruptStatus();

        const uint16_t fifoCount = getFifoCount();

        if (fifoCount > 0)
        {
            data = getFifoValues();
        }
    }
}
