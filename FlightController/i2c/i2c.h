/*
 * i2c.h
 *
 *  Created on: Apr 2, 2022
 *      Author: Akeman
 */

#ifndef I2C_I2C_H_
#define I2C_I2C_H_

typedef enum I2CNames
{
    MPU6050_I2C0 = 0,

    i2cCount
} iANARO_I2CNames;

void initI2C(void);

#endif /* I2C_I2C_H_ */
