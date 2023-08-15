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
#include "qcom/qcom.h"
#include "pwm/pwm.h"
#include "utils/logger.h"

#define TCPPACKETSIZE 256
#define NUMTCPWORKERS 3

Mailbox_Handle tcpMailbox;
Mailbox_Struct tcpMailboxStruct;
tcpMessageObject tcpMailboxBuffer[TCP_MAILBOXSLOTS + 1];

Clock_Struct tcpMsgClockStruct;
Clock_Handle tcpMsgClockHandle;

void tcpMessageClockFxn(void)
{
    qMessage msg;

    memset(&msg, 0, sizeof(qMessage));

    const MPU6050_Data data = getMPU6050Data();
    const RollPitchYawInRad rpy = getEulerAngles();

    tcpMessageAngles foo;
    memcpy(&foo.rawData, &data, sizeof(MPU6050_Data));
    memcpy(&foo.rpy, &rpy, sizeof(RollPitchYawInRad));

    memcpy(msg.payload, &foo, sizeof(tcpMessageAngles));

    msg.header.messageID = QMSGID_IMU;
    msg.header.length = sizeof(tcpMessageAngles);

    Mailbox_post(tcpMailbox, &msg, BIOS_NO_WAIT);
}

static void fillHeader(qMessage* msg)
{
    static uint8_t frameCounter;

    msg->header.syncByte = QSYNC;
    msg->header.frameCounter = frameCounter++;
    msg->header.length += sizeof(qHeader);
}

void tcpWorker(UArg arg0, UArg arg1)
{
    qMessage msg;
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

        bytesSent = send(clientfd, &msg, msg.header.length, 0);
    }

    Clock_stop(tcpMsgClockHandle);

    close(clientfd);
}

void tcpRx(UArg arg0)
{
    qMessage msg;
    int clientfd = (int)arg0;
    int bytesRcvd;
    char tcpBuffer[TCPPACKETSIZE];
    qParserStatusStruct parserStatus;

    initQParser(&parserStatus);

    while ((bytesRcvd = recv(clientfd, tcpBuffer, TCPPACKETSIZE, 0)) > 0)
    {
        for (int byteIndex = 0; byteIndex < bytesRcvd; byteIndex++)
        {
            const int parseResult = parseQ(tcpBuffer[byteIndex], &msg, &parserStatus);

            if (parseResult == 1)
            {
                if (msg.header.messageID == QMSGID_PWM)
                {
                    qMessagePWM* pwm = (qMessagePWM*)msg.payload;

                    setPWM1Percent(pwm->percentagePWM1);
                    setPWM2Percent(pwm->percentagePWM2);

//                    char buffer[64];
//                    snprintf(buffer, sizeof(buffer), "PWM set to %u", pwm->percentage);
//                    printLog(buffer, INFOMSG);
                }
            }
        }

        if (bytesRcvd <= 0)
        {
            break;
        }
    }
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
        goto shutdown;
    }

    while ((clientfd = accept(server, (struct sockaddr *)&clientAddr, &addrlen)) != -1)
    {
        Task_Params_init(&taskParams);
        taskParams.arg0 = (UArg)clientfd;
        taskParams.stackSize = 1280;
        taskHandle = Task_create((Task_FuncPtr)tcpWorker, &taskParams, NULL);

        Task_Params_init(&taskParams);
        taskParams.arg0 = (UArg)clientfd;
        taskParams.stackSize = 4096;
        Task_create((Task_FuncPtr)tcpRx, &taskParams, NULL);

        if (taskHandle == NULL)
        {
            close(clientfd);
        }

        addrlen = sizeof(clientAddr);
    }

shutdown:
    if (server > 0)
    {
        close(server);
    }
}
