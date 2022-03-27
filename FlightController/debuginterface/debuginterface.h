/*
 * debuginterface.h
 *
 *  Created on: Mar 27, 2022
 *      Author: Akeman
 */

#ifndef DEBUGINTERFACE_DEBUGINTERFACE_H_
#define DEBUGINTERFACE_DEBUGINTERFACE_H_

#include <ti/sysbios/knl/Mailbox.h>

#define MAILBOXSIZE 128
#define UART_MAXSTRLEN 128
#define UART_MAILBOXFULL_ERRORFLAG 0x01
#define MAILBOXSLOTS 50

typedef struct __attribute__((__packed__))
{
    char msg[MAILBOXSIZE];
    int size;
} DebugMessage;

typedef struct __attribute__((__packed__))
{
    Mailbox_MbxElem q;
    DebugMessage msg;
} DebugMessageObject;

typedef enum
{
    CMD_NOT_SET = -1,
    CMD_OK = 0,
    CMD_BAD_COMMAND,
    CMD_PARAM_OOB,
    CMD_PARAM_BAD,
    CMD_EMPTY,
} cmdState;

typedef enum
{
    GET,
    SET,
    CMD,
    LOG,
    ELSE
} cmdType;

typedef enum
{
    SHOW_ITEM,
    HIDE_ITEM
} cmdShow;

typedef const struct
{
    char* command;
    cmdType commandType;
    char* help;
    int minarg;
    cmdShow show;
    cmdState (*func)(const char* const argv[], const int argc);
}  cmdItem;

extern Mailbox_Handle debugMailbox;
extern Mailbox_Struct debugMailboxStruct;
extern DebugMessageObject MailboxBuffer[MAILBOXSLOTS + 1];

void writeDebug(const char* message);
void writeDebugTaskFxn(void);
void readDebugTaskFxn(void);

#endif /* DEBUGINTERFACE_DEBUGINTERFACE_H_ */
