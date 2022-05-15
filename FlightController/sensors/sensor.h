/*
 * sensor.h
 *
 *  Created on: Apr 2, 2022
 *      Author: Akeman
 */

#ifndef SENSORS_SENSOR_H_
#define SENSORS_SENSOR_H_

#include <ti/sysbios/gates/GateSwi.h>

#include "sensors/mpu6050/mpu6050.h"

typedef struct
{
    double roll;
    double pitch;
    double yaw;
} RollPitchYawInRad;

extern GateSwi_Handle sensorDataSwiGateHandle;
extern GateSwi_Struct sensorDataSwiGateStruct;

void sensorTaskFxn(void);
MPU6050_Data getMPU6050Data(void);
MPU6050_Comstats getMPU6050Comstats(void);
RollPitchYawInRad getEulerAngles(void);

#endif /* SENSORS_SENSOR_H_ */
