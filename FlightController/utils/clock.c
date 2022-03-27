/*
 * clock.c
 *
 *  Created on: Sep 1, 2020
 *      Author: akos
 */

#include <stdint.h>

#include <ti/sysbios/knl/Clock.h>

static unsigned long long msTicks;
Clock_Struct msTicksClockStruct;

inline unsigned long long getMsTicks(void)
{
    return msTicks;
}

void msTicksFxn(void)
{
    msTicks++;
}
