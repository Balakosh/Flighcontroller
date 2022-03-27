/*
 * uart.c
 *
 *  Created on: Mar 27, 2022
 *      Author: Akeman
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#include <xdc/std.h>

#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_gpio.h>

#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>
#include <driverlib/uart.h>

#include <ti/drivers/UART.h>
#include <ti/drivers/uart/UARTTiva.h>

#include "uart/uart.h"

UART_Handle debugUARTHandle = NULL;

UARTTiva_Object uartTivaObjects[UART_COUNT];
unsigned char uartTivaRingBuffer[UART_COUNT][512];

const UARTTiva_HWAttrs uartTivaHWAttrs[UART_COUNT] =
{
    {
        .baseAddr = UART0_BASE,
        .intNum = INT_UART0,
        .intPriority = (~0),
        .flowControl = 0,
        .ringBufPtr  = uartTivaRingBuffer[0],
        .ringBufSize = sizeof(uartTivaRingBuffer[0])
    },
};

const UART_Config UART_config[] =
{
    {
        .fxnTablePtr = &UARTTiva_fxnTable,
        .object = &uartTivaObjects[0],
        .hwAttrs = &uartTivaHWAttrs[0]
    },
    {NULL, NULL, NULL}
};

void initUART(void)
{
    // Debug Console
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)) {};

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UART_init();
}

int openDebugUART(void)
{
    if(debugUARTHandle == NULL)
    {
        UART_Params uartParams;
        UART_Params_init(&uartParams);

        uartParams.readMode       = UART_MODE_BLOCKING;
        uartParams.writeMode      = UART_MODE_BLOCKING;
        uartParams.readDataMode   = UART_DATA_BINARY;
        uartParams.writeDataMode  = UART_DATA_BINARY;
        uartParams.baudRate       = 9600;
        uartParams.readReturnMode = UART_RETURN_FULL;
        uartParams.readEcho       = UART_ECHO_OFF;
        uartParams.dataLength     = UART_LEN_8;
        uartParams.stopBits       = UART_STOP_ONE;
        uartParams.parityType     = UART_PAR_NONE;

        debugUARTHandle = UART_open(UART0, &uartParams);

        if(debugUARTHandle == NULL)
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    return 1;
}
