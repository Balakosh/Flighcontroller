/*
 * logger.h
 *
 *  Created on: Nov 2, 2020
 *      Author: akos
 */

#ifndef UTILS_LOGGER_H_
#define UTILS_LOGGER_H_

#define FOREACH_LogMessageType(LogMessageType) \
        LogMessageType(INFOMSG) \
        LogMessageType(WARNINGMSG) \
        LogMessageType(ERRORMSG) \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum {
    FOREACH_LogMessageType(GENERATE_ENUM)
}LogMessageType;

static const char *LogMessageType_STRING[] = {
    FOREACH_LogMessageType(GENERATE_STRING)
};

void printLog(const char* message, LogMessageType type);

#endif /* UTILS_LOGGER_H_ */
