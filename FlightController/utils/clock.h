/*
 * clock.h
 *
 *  Created on: Sep 1, 2020
 *      Author: akos
 */

#ifndef UTILS_CLOCK_H_
#define UTILS_CLOCK_H_

#include <ti/sysbios/knl/Clock.h>

extern Clock_Struct msTicksClockStruct;

void msTicksFxn(void);
unsigned long long getMsTicks(void);

#endif /* UTILS_CLOCK_H_ */
