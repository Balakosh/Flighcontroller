/*
 * logger.c
 *
 *  Created on: Nov 2, 2020
 *      Author: akos
 */

#include <string.h>
#include <stdio.h>

#include "utils/logger.h"
#include "utils/clock.h"
#include "debuginterface/debuginterface.h"

void printLog(const char* const message, const LogMessageType type)
{
    char buffer[1024];

    snprintf(buffer, sizeof(buffer), "%llu %s %s\r\n", getMsTicks(), LogMessageType_STRING[type], message);
    //snprintf(buffer, sizeof(buffer), "%s\r", message);

    writeDebug(buffer);
}
