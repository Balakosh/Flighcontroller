/*
 * gpio.h
 *
 *  Created on: Mar 27, 2022
 *      Author: Akeman
 */

#ifndef GPIO_GPIO_H_
#define GPIO_GPIO_H_

typedef enum GPIOName
{
    // Inputs
    EK_TM4C1294XL_USR_SW1 = 0,
    EK_TM4C1294XL_USR_SW2,

    // Outputs
    EK_TM4C1294XL_D1,
    EK_TM4C1294XL_D2,
    foo,

    EK_TM4C1294XL_GPIOCOUNT
} GPIOName;

void initGPIO(void);
void createGPIOCallbacks(void);
void toggleLEDD1(void);

#endif /* GPIO_GPIO_H_ */
