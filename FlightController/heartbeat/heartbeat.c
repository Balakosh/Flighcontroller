/*
 * heartbeat.c
 *
 *  Created on: Mar 27, 2022
 *      Author: Akeman
 */

#include <ti/sysbios/knl/Task.h>

#include "gpio/gpio.h"

void heartBeatTaskFxn(void)
{
    while (1)
    {
        toggleLEDD1();
        Task_sleep(1000);
    }
}
