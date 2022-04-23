/*
 * sensor.h
 *
 *  Created on: Apr 2, 2022
 *      Author: Akeman
 */

#ifndef SENSORS_SENSOR_H_
#define SENSORS_SENSOR_H_

#include "sensors/mpu6050/mpu6050.h"

typedef struct
{
    double roll;
    double pitch;
    double yaw;
} RollPitchYawInRad;

void sensorTaskFxn(void);
MPU6050_Data getMPU6050Data(void);
RollPitchYawInRad getEulerAngles(void);

#endif /* SENSORS_SENSOR_H_ */
