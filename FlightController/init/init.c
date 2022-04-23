/*
 * init.c
 *
 *  Created on: Mar 27, 2022
 *      Author: Akeman
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <inc/hw_types.h>
#include <inc/hw_memmap.h>

#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>

#include <ti/drivers/GPIO.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>

#include "tasks/tasks.h"
#include "gpio/gpio.h"
#include "uart/uart.h"
#include "debuginterface/debuginterface.h"
#include "utils/clock.h"
#include "version/version.h"
#include "utils/logger.h"
#include "pwm/pwm.h"
#include "esc/esc.h"
#include "i2c/i2c.h"
#include "sensors/mpu6050/mpu6050.h"

char resetCauseString[128];
uint32_t resetCause;

static void initClocks(void)
{
    Clock_Params clockParams;

    Clock_Params_init(&clockParams);
    clockParams.period = 1;
    clockParams.startFlag = true;
    clockParams.instance->name = "msTick";

    Clock_construct(&msTicksClockStruct, (Clock_FuncPtr)msTicksFxn, 1, &clockParams);

    Clock_Params_init(&clockParams);
    clockParams.period = 20;
    clockParams.startFlag = false;
    clockParams.instance->name = "escCalibrate";

    Clock_construct(&escCalibrationClockStruct, (Clock_FuncPtr)escCalibrationClockFxn, 1, &clockParams);
    escCalibrationClockHandle = Clock_handle(&escCalibrationClockStruct);

    Clock_Params_init(&clockParams);
    clockParams.period = 1000;
    clockParams.startFlag = false;
    clockParams.instance->name = "dataLog";

    Clock_construct(&dataLogClockStruct, (Clock_FuncPtr)dataLogClockFxn, 1, &clockParams);
    dataLogClockHandle = Clock_handle(&dataLogClockStruct);
}

static void initMailboxes(void)
{
    Mailbox_Params mailboxParams;
    Mailbox_Params_init(&mailboxParams);

    mailboxParams.buf = MailboxBuffer;
    mailboxParams.bufSize = sizeof(MailboxBuffer);
    mailboxParams.instance->name = (xdc_String)"debugMailbox";
    Mailbox_construct(&debugMailboxStruct, sizeof(DebugMessage), MAILBOXSLOTS, &mailboxParams, NULL);
    debugMailbox = Mailbox_handle(&debugMailboxStruct);
}

static void initPeripherals(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOR);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOS);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOT);
}

static int getResetCause(void)
{
    char* externalReset = "";
    char* poweronReset = "";
    char* brownoutReset = "";
    char* watchdog0Reset = "";
    char* watchdog1Reset = "";
    char* softwareReset = "";
    char* hssrReset = "";
    char* moscFailReset = "";

    memset(resetCauseString, 0, sizeof(resetCauseString));

    resetCause = SysCtlResetCauseGet();

    if ((resetCause & SYSCTL_CAUSE_EXT) == SYSCTL_CAUSE_EXT)
    {
        externalReset = " External reset";
    }

    if ((resetCause & SYSCTL_CAUSE_BOR) == SYSCTL_CAUSE_BOR)
    {
        brownoutReset = " Brown-Out reset";
    }

    if ((resetCause & SYSCTL_CAUSE_WDOG0) == SYSCTL_CAUSE_WDOG0)
    {
        watchdog0Reset = " Watchdog Timer 0 reset";
    }

    if ((resetCause & SYSCTL_CAUSE_SW) == SYSCTL_CAUSE_SW)
    {
        softwareReset = " Software reset";
    }

    if ((resetCause & SYSCTL_CAUSE_WDOG1) == SYSCTL_CAUSE_WDOG1)
    {
        watchdog1Reset = " Watchdog Timer 1 reset";
    }

    if ((resetCause & SYSCTL_CAUSE_HSRVREQ) == SYSCTL_CAUSE_HSRVREQ)
    {
        hssrReset = " HSSR reset";
    }

    SysCtlResetCauseClear(SysCtlResetCauseGet());

    snprintf(resetCauseString, sizeof(resetCauseString), "%s%s%s%s%s%s%s%s",
             externalReset, poweronReset, brownoutReset, watchdog0Reset, watchdog1Reset, softwareReset, hssrReset, moscFailReset);

    return resetCause;
}

void initSemaphores(void)
{
    Semaphore_Params params;

    Semaphore_Params_init(&params);
    params.mode = Semaphore_Mode_BINARY;

    Semaphore_construct(&mpu6050InterruptSemaphoreStruct, 0, &params);
    mpu6050InterruptSemaphoreHandle = Semaphore_handle(&mpu6050InterruptSemaphoreStruct);
}

void init(void)
{
    initPeripherals();
    initGPIO();
    initUART();
    openDebugUART();
    initPWM();
    initI2C();

    initTasks();
    initMailboxes();
    initClocks();
    initSemaphores();

    getResetCause();

    xdc_runtime_Types_FreqHz freq;
    BIOS_getCpuFreq((xdc_runtime_Types_FreqHz*)&freq);

    char startMessage[128];
    snprintf(startMessage, sizeof(startMessage), "+++ Flighcontroller %u.%u.%u program start . reset cause was%s +++",
             getVersionMajor(), getVersionMinor(), getVersionPatch(), resetCauseString);
    printLog(startMessage, INFOMSG);

    snprintf(startMessage, sizeof(startMessage), "CPU freq=%d TI-RTOS Version %#x", freq.lo, BIOS_version);
    printLog(startMessage, INFOMSG);
}
