/*
 * debuginterface.c
 *
 *  Created on: Mar 27, 2022
 *      Author: Akeman
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

#include <ti/drivers/UART.h>

#include "version/version.h"
#include "debuginterface.h"
#include "utils/logger.h"
#include "uart/uart.h"
#include "pwm/pwm.h"

char buffer[255];
Mailbox_Handle debugMailbox;
Mailbox_Struct debugMailboxStruct;
DebugMessageObject MailboxBuffer[MAILBOXSLOTS + 1];

// GET
static cmdState getVersion(const char* const argv[], const int argc);

// GET
static cmdState debugSetPWM(const char* const argv[], const int argc);

// CMD
static cmdState cmdTest(const char* const argv[], const int argc);

static const cmdItem DEBUG_CMD_ARRAY [] =
{
    // GET
    {"version", GET, "get version", 2, SHOW_ITEM, getVersion},

    // SET
    {"pwm", SET, "set pwm <dutyCycleInPercent>", 2, SHOW_ITEM, debugSetPWM},

    // CMD
    {"test", CMD, "cmd test", 2,  SHOW_ITEM, cmdTest}

    // LOG
};

static cmdState debugSetPWM(const char* const argv[], const int argc)
{
    const uint32_t pwmDutyCycleInPercent = atoi(argv[2]);

    setPWM(pwmDutyCycleInPercent);

    snprintf(buffer, sizeof(buffer), "PWM set to %u", pwmDutyCycleInPercent);
    printLog(buffer, INFOMSG);

    return CMD_OK;
}

static cmdState getVersion(const char* const argv[], const int argc)
{
    char buffer[512];

    snprintf(buffer, sizeof(buffer), "Version: %d.%d.%d compiled on %s at %s. GIT %s",
             getVersionMajor(), getVersionMinor(), getVersionPatch(), __DATE__, __TIME__, getGitInfo());

    printLog(buffer, INFOMSG);

    return CMD_OK;
}

static cmdState cmdTest(const char* const argv[], const int argc)
{
    return CMD_OK;
}

static void postMailbox(DebugMessage* const debugMessage)
{
    if (!Mailbox_post(debugMailbox, debugMessage, BIOS_NO_WAIT))
    {
        const char* const errorString = "debugMailbox timeout! Mailbox is full!\r";
        DebugMessage mailboxFullMsg;
        const int length = snprintf(mailboxFullMsg.msg, sizeof(mailboxFullMsg.msg), errorString);
        mailboxFullMsg.size = length;

        Mailbox_post(debugMailbox, &mailboxFullMsg, BIOS_WAIT_FOREVER);
    }
}

void writeDebug(const char* const message)
{
    DebugMessage debugMessage;

    const int size = strlen(message);

    const int subStringCount = size / MAILBOXSIZE;
    const int leftOver = size %  MAILBOXSIZE;

    for (int i = 0; i < subStringCount; i++)
    {
        debugMessage.size = MAILBOXSIZE;
        memcpy((void *)&debugMessage.msg, message + (i * MAILBOXSIZE), debugMessage.size);
        postMailbox(&debugMessage);
    }

    if (leftOver > 0)
    {
        debugMessage.size = leftOver;
        memcpy((void *)&debugMessage.msg, message + (size - leftOver), debugMessage.size);
        postMailbox(&debugMessage);
    }
}

void writeDebugTaskFxn(void)
{
    DebugMessage debugMsg;

    while (1)
    {
        Mailbox_pend(debugMailbox, &debugMsg, BIOS_WAIT_FOREVER);

        UART_write(debugUARTHandle, &debugMsg.msg, debugMsg.size);
    }
}

static unsigned int getCommandArraySize(void)
{
    return sizeof(DEBUG_CMD_ARRAY) / sizeof(cmdItem);
}

static char* commandTypeToString(const cmdType commandType)
{
    switch (commandType)
    {
        case GET:
            return "get";

        case SET:
            return "set";

        case CMD:
            return "cmd";

        case LOG:
            return "log";

        default:
            return "get";
    }
}

void printSetHelpText(bool helpAdminText)
{
    char helpText[512] = "Available SET commands: ";

    const unsigned int arraySize = getCommandArraySize();

    for (int i = 0; i < arraySize; ++i)
    {
        if ((DEBUG_CMD_ARRAY[i].commandType == SET) && ((DEBUG_CMD_ARRAY[i].show == SHOW_ITEM) || helpAdminText))
        {
            if (strlen(helpText) < (strlen(helpText) + strlen(DEBUG_CMD_ARRAY[i].command) + 4))
            {
                strncat(helpText, "[", 1);
                strncat(helpText, DEBUG_CMD_ARRAY[i].command, strlen(DEBUG_CMD_ARRAY[i].command));
                strncat(helpText, "] ", 2);
            }
            else
            {
                break;
            }
        }
    }

    printLog(helpText, INFOMSG);
}

void printGetHelpText(bool helpAdminText)
{
    char helpText[512] = "Available GET commands: ";

    const unsigned int arraySize = getCommandArraySize();

    for (int i = 0; i < arraySize; ++i)
    {
        if ((DEBUG_CMD_ARRAY[i].commandType == GET) && ((DEBUG_CMD_ARRAY[i].show == SHOW_ITEM) || helpAdminText))
        {
            if (strlen(helpText) < (strlen(helpText) + strlen(DEBUG_CMD_ARRAY[i].command) + 4))
            {
                strncat(helpText, "[", 1);
                strncat(helpText, DEBUG_CMD_ARRAY[i].command, strlen(DEBUG_CMD_ARRAY[i].command));
                strncat(helpText, "] ", 2);
            }
            else
            {
                break;
            }
        }
    }

    printLog(helpText, INFOMSG);
}

void printCmdHelpText(bool helpAdminText)
{
    char helpText[512] = "Available CMD commands: ";

    const unsigned int arraySize = getCommandArraySize();

    for (int i = 0; i < arraySize; ++i)
    {
        if ((DEBUG_CMD_ARRAY[i].commandType == CMD) && ((DEBUG_CMD_ARRAY[i].show == SHOW_ITEM) || helpAdminText))
        {
            if (strlen(helpText) < (strlen(helpText) + strlen(DEBUG_CMD_ARRAY[i].command) + 4))
            {
                strncat(helpText, "[", 1);
                strncat(helpText, DEBUG_CMD_ARRAY[i].command, strlen(DEBUG_CMD_ARRAY[i].command));
                strncat(helpText, "] ", 2);
            }
            else
            {
                break;
            }
        }
    }

    printLog(helpText, INFOMSG);
}

void printLogHelpText(bool helpAdminText)
{
    char helpText[512] = "Available LOG commands: ";

    const unsigned int arraySize = getCommandArraySize();

    for (int i = 0; i < arraySize; ++i)
    {
        if ((DEBUG_CMD_ARRAY[i].commandType == LOG) && ((DEBUG_CMD_ARRAY[i].show == SHOW_ITEM) || helpAdminText))
        {
            if (strlen(helpText) < (strlen(helpText) + strlen(DEBUG_CMD_ARRAY[i].command) + 4))
            {
                strncat(helpText, "[", 1);
                strncat(helpText, DEBUG_CMD_ARRAY[i].command, strlen(DEBUG_CMD_ARRAY[i].command));
                strncat(helpText, "] ", 2);
            }
            else
            {
                break;
            }
        }
    }

    printLog(helpText, INFOMSG);
}

void printElseHelpText(bool helpAdminText)
{
    char helpText[512] = "Other commands: ";

    const unsigned int arraySize = getCommandArraySize();

    for (int i = 0; i < arraySize; ++i)
    {
        if ((DEBUG_CMD_ARRAY[i].commandType == ELSE) && ((DEBUG_CMD_ARRAY[i].show == SHOW_ITEM) || helpAdminText))
        {
            if (strlen(helpText) < (strlen(helpText) + strlen(DEBUG_CMD_ARRAY[i].command) + 4))
            {
                strncat(helpText, "[", 1);
                strncat(helpText, DEBUG_CMD_ARRAY[i].command, strlen(DEBUG_CMD_ARRAY[i].command));
                strncat(helpText, "] ", 2);
            }
            else
            {
                break;
            }
        }
    }

    printLog(helpText, INFOMSG);
}

void printHelpText(bool helpAdminText)
{
    printGetHelpText(helpAdminText);
    printSetHelpText(helpAdminText);
    printCmdHelpText(helpAdminText);
    printLogHelpText(helpAdminText);
    printElseHelpText(helpAdminText);
}

static cmdState cmdSearch(char* argv[32], int argc)
{
    cmdState commandState = CMD_BAD_COMMAND;

    if (argc < 1)
    {
        return CMD_BAD_COMMAND;
    }

    const unsigned int arraySize = getCommandArraySize();

    for (int i = 0; i < arraySize; ++i)
    {
        const int commandStringLength = strlen(DEBUG_CMD_ARRAY[i].command);
        const char* const cmdType = commandTypeToString(DEBUG_CMD_ARRAY[i].commandType);

        if ((argc > 1) && (strncmp(DEBUG_CMD_ARRAY[i].command, argv[1], commandStringLength) == 0) && (strncmp(cmdType, argv[0], 3) == 0))
        {
            if (argc >= DEBUG_CMD_ARRAY[i].minarg)
            {
                commandState = DEBUG_CMD_ARRAY[i].func(argv, argc);

                if ((commandState == CMD_OK) && (DEBUG_CMD_ARRAY[i].commandType == LOG))
                {
                    printLog("setting log.\r", INFOMSG);
                }
            }
            else
            {
                commandState = CMD_PARAM_OOB;
            }

            if ((commandState != CMD_OK) && (commandState != CMD_NOT_SET))
            {
                printLog(DEBUG_CMD_ARRAY[i].help, INFOMSG);
            }

            break;
        }
        else if ((strncmp(DEBUG_CMD_ARRAY[i].command, argv[0], commandStringLength) == 0) && (DEBUG_CMD_ARRAY[i].commandType == ELSE))
        {
            commandState = DEBUG_CMD_ARRAY[i].func(argv, argc);
        }
    }

    if ((argc >= 1) && (commandState == CMD_BAD_COMMAND))
    {
        if ((strncmp("help", argv[0], 4) == 0) || (strncmp("?", argv[0], 3) == 0))
        {
            bool helpAdminText = false;

            if ((argc == 2) && (strncmp("imar", argv[1], 4) == 0))
            {
                helpAdminText = true;
            }

            printHelpText(helpAdminText);
            commandState = CMD_OK;
        }
    }

    return commandState;
}

void readDebugTaskFxn(void)
{
    char nextChar;
    bool uart_str_complete = false;
    unsigned int uart_str_count = 0;
    char uart_string[UART_MAXSTRLEN + 1] = "";
    bool discardWhitespaces = false;

    while (1)
    {
        const int bytesRead = UART_read(debugUARTHandle, &nextChar, sizeof(nextChar));

        if ((bytesRead == 1) && (!uart_str_complete))
        {
            if (nextChar != '\n' && nextChar != '\r' && nextChar != '\t' && uart_str_count < UART_MAXSTRLEN)
            {
                discardWhitespaces = false;
                uart_string[uart_str_count] = nextChar;
                uart_str_count++;
            }
            else if (discardWhitespaces)
            {
            }
            else
            {
                uart_string[uart_str_count] = '\0';
                uart_str_complete = true;
                discardWhitespaces = true;

                // convert to lowercase
                for(int i = 0; uart_string[i]; i++)
                {
                    uart_string[i] = tolower(uart_string[i]);
                }
            }

            if (uart_str_complete == 1)
            {
                const char s[2] = " ";
                char *token;
                int argc = 0;
                char* argv[32];

                token = strtok(uart_string, s);

                while ((token != NULL) && (argc < 32))
                {
                    argv[argc] = token;
                    argc++;
                    token = strtok(NULL, s);
                }

                const cmdState cmdSearchResult = cmdSearch(argv, argc);

                switch (cmdSearchResult)
                {
                    case CMD_NOT_SET:
                    {
                        printLog("cmd not set!", ERRORMSG);
                        break;
                    }
                    case CMD_BAD_COMMAND:
                    {
                        printLog("bad command!", ERRORMSG);
                        break;
                    }
                    case CMD_PARAM_OOB:
                    {
                        printLog("parameters out of bounds!", ERRORMSG);
                        break;
                    }
                    case CMD_PARAM_BAD:
                    {
                        printLog("bad parameter!", ERRORMSG);
                        break;
                    }
                    case CMD_EMPTY:
                    {
                        printLog("cmd empty!", ERRORMSG);
                        break;
                    }
                }

                uart_str_complete = 0;
                uart_str_count = 0;
            }
        }
    }
}
