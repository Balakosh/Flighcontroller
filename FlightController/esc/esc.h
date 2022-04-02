/*
 * esc.h
 *
 *  Created on: Apr 2, 2022
 *      Author: Akeman
 */

#ifndef ESC_ESC_H_
#define ESC_ESC_H_

#include <ti/sysbios/knl/Clock.h>

extern Clock_Struct escCalibrationClockStruct;
extern Clock_Handle escCalibrationClockHandle;

void calibrateESC(void);
void escCalibrationClockFxn(void);

#endif /* ESC_ESC_H_ */
