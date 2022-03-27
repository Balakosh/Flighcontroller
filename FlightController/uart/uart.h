/*
 * uart.h
 *
 *  Created on: Mar 27, 2022
 *      Author: Akeman
 */

#ifndef UART_UART_H_
#define UART_UART_H_

#include <ti/sysbios/knl/Mailbox.h>

#include <ti/drivers/UART.h>

typedef enum
{
    UART0 = 0,

    UART_COUNT
} UARTNames;


extern UART_Handle debugUARTHandle;

int openDebugUART(void);
void initUART();

#endif /* UART_UART_H_ */
