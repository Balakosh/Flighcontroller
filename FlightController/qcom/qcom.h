/*
 * qcom.h
 *
 *  Created on: May 27, 2022
 *      Author: Akeman
 */

#ifndef QCOM_QCOM_H_
#define QCOM_QCOM_H_

#include <stdint.h>

#define QMSG_MAX_PAYLOAD_SIZE 128
#define QSYNC 0x7e

typedef struct __attribute__ ((__packed__)) {
    uint8_t syncByte;
    uint8_t messageID;
    uint8_t frameCounter;
    uint8_t reservedByte;
    uint16_t length;
} qHeader;

typedef struct __attribute__ ((__packed__)) {
    qHeader header;
    unsigned char payload[QMSG_MAX_PAYLOAD_SIZE];
} qMessage;

typedef enum {
    SYNC,
    msgID,
    frameCounter,
    reservedByte,
    messageLength,
    payload
} qParserStates;

typedef struct __attribute__ ((__packed__)) {
    qParserStates ixComParserState;
    int ixComParserSubState;
    int payloadCounter;
} qParserStatusStruct;

int parseQ(uint8_t data, qMessage* ethMessage, qParserStatusStruct* parserStatus);

#endif /* QCOM_QCOM_H_ */
