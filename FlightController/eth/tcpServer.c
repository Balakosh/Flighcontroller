/*
 * tcpServer.c
 *
 *  Created on: May 15, 2022
 *      Author: Akeman
 */

#include <string.h>
#include <stdio.h>

#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Clock.h>

#include <ti/drivers/GPIO.h>

#include <sys/socket.h>

#include "tcpServer.h"
#include "sensors/sensor.h"

#define TCPPACKETSIZE 256
#define NUMTCPWORKERS 3

Mailbox_Handle tcpMailbox;
Mailbox_Struct tcpMailboxStruct;
tcpMessageObject tcpMailboxBuffer[TCP_MAILBOXSLOTS + 1];

Clock_Struct tcpMsgClockStruct;
Clock_Handle tcpMsgClockHandle;

const uint8_t SYNC = 0x7e;
static uint8_t frameCounter;

void tcpMessageClockFxn(void)
{
    TcpMessage msg;

    memset(&msg, 0, sizeof(TcpMessage));

    const MPU6050_Data data = getMPU6050Data();
//    memcpy(msg.payload, &data, sizeof(MPU6050_Data));

    //const RollPitchYawInRad rpy = getEulerAngles();
    const RollPitchYawInRad rpy = { 1.0, 2.0, 3.0 };
//    memcpy(&msg.payload[sizeof(MPU6050_Data)], &rpy, sizeof(RollPitchYawInRad));

    tcpMessageAngles foo;
    memcpy(&foo.rawData, &data, sizeof(MPU6050_Data));
    memcpy(&foo.rpy, &rpy, sizeof(RollPitchYawInRad));

    memcpy(msg.payload, &foo, sizeof(tcpMessageAngles));

    //msg.size = sizeof(MPU6050_Data) + sizeof(RollPitchYawInRad);
    msg.size = sizeof(tcpMessageAngles);

    Mailbox_post(tcpMailbox, &msg, BIOS_NO_WAIT);
}

static void fillHeader(TcpMessage* msg)
{
    msg->header.sync = SYNC;
    msg->header.frameCounter = frameCounter++;
    msg->header.size = sizeof(TcpMessageHeader) + msg->size;
}

void tcpWorker(UArg arg0, UArg arg1)
{
    TcpMessage msg;
    int clientfd = (int)arg0;
    int bytesSent = 1;

//    while ((bytesRcvd = recv(clientfd, buffer, TCPPACKETSIZE, 0)) > 0)
//    {
//        bytesSent = send(clientfd, buffer, bytesRcvd, 0);
//
//        if (bytesSent < 0 || bytesSent != bytesRcvd)
//        {
//            System_printf("Error: send failed.\n");
//            break;
//        }
//    }

    Clock_start(tcpMsgClockHandle);

    while (bytesSent > 0)
    {
        Mailbox_pend(tcpMailbox, &msg, BIOS_WAIT_FOREVER);

        fillHeader(&msg);

        bytesSent = send(clientfd, &msg, msg.header.size, 0);
    }

    Clock_stop(tcpMsgClockHandle);

    close(clientfd);
}

void tcpHandler(UArg arg0, UArg arg1)
{
    int                status;
    int                clientfd;
    int                server;
    struct sockaddr_in localAddr;
    struct sockaddr_in clientAddr;
    int                optval;
    int                optlen = sizeof(optval);
    socklen_t          addrlen = sizeof(clientAddr);
    Task_Handle        taskHandle;
    Task_Params        taskParams;
    Error_Block        eb;

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == -1)
    {
        goto shutdown;
    }

    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(arg0);

    status = bind(server, (struct sockaddr *)&localAddr, sizeof(localAddr));

    if (status == -1)
    {
        goto shutdown;
    }

    status = listen(server, NUMTCPWORKERS);
    if (status == -1)
    {
        goto shutdown;
    }

    optval = 1;
    if (setsockopt(server, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0)
    {
        System_printf("Error: setsockopt failed\n");
        goto shutdown;
    }

    while ((clientfd = accept(server, (struct sockaddr *)&clientAddr, &addrlen)) != -1)
    {
        Error_init(&eb);

        Task_Params_init(&taskParams);
        taskParams.arg0 = (UArg)clientfd;
        taskParams.stackSize = 1280;
        taskHandle = Task_create((Task_FuncPtr)tcpWorker, &taskParams, &eb);

        if (taskHandle == NULL)
        {
            System_printf("Error: Failed to create new Task\n");
            close(clientfd);
        }

        addrlen = sizeof(clientAddr);
    }

    System_printf("Error: accept failed.\n");

shutdown:
    if (server > 0)
    {
        close(server);
    }
}
