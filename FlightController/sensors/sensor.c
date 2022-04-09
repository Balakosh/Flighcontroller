/*
 * sensor.c
 *
 *  Created on: Apr 2, 2022
 *      Author: Akeman
 */

#include <ti/sysbios/knl/Task.h>

#include <ti/drivers/I2C.h>

#include "i2c/i2c.h"
#include "mpu6050/mpu6050.h"

void sensorTaskFxn(void)
{
    initMPU6050();

    while (1)
    {
        Task_sleep(1000);

        readAccelerometer();

        Task_sleep(10);

        readGyroscope();

        Task_sleep(10);

        readTemperature();
    }
}
