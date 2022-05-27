/*
 * tcpServer.h
 *
 *  Created on: May 15, 2022
 *      Author: Akeman
 */

#ifndef ETH_TCPSERVER_H_
#define ETH_TCPSERVER_H_

#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Clock.h>

#define TCP_MAILBOXSIZE 128
#define TCP_MAILBOXSLOTS 50

#include "sensors/sensor.h"

typedef struct __attribute__((__packed__))
{
    uint8_t sync;
    uint8_t frameCounter;
    uint32_t size;
} TcpMessageHeader;

typedef struct __attribute__((__packed__))
{
    TcpMessageHeader header;
    uint8_t payload[TCP_MAILBOXSIZE];
    uint32_t size;
} TcpMessage;

typedef struct __attribute__((__packed__))
{
    Mailbox_MbxElem q;
    TcpMessage payload;
} tcpMessageObject;

typedef struct __attribute__((__packed__))
{
    MPU6050_Data rawData;
    RollPitchYawInRad rpy;
} tcpMessageAngles;

extern Mailbox_Handle tcpMailbox;
extern Mailbox_Struct tcpMailboxStruct;
extern tcpMessageObject tcpMailboxBuffer[TCP_MAILBOXSLOTS + 1];

extern Clock_Struct tcpMsgClockStruct;
extern Clock_Handle tcpMsgClockHandle;

void tcpHandler(UArg arg0, UArg arg1);
void tcpMessageClockFxn(void);

#endif /* ETH_TCPSERVER_H_ */
