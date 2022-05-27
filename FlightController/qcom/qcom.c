/*
 * qcom.c
 *
 *  Created on: May 27, 2022
 *      Author: Akeman
 */

#include "qcom/qcom.h"

void initQParser(qParserStatusStruct* parserStatus)
{
    parserStatus->subState = 0;
    parserStatus->payloadCounter = 0;
    parserStatus->state = SYNC;
}

int parseQ(const uint8_t data, qMessage* msg, qParserStatusStruct* parserStatus)
{
    int result = 0;

    switch (parserStatus->state)
    {
        case SYNC:
        {
            if (data == QSYNC)
            {
                msg->header.syncByte = data;
                parserStatus->state = msgID;
            }
            break;
        }
        case msgID:
        {
            msg->header.messageID = data;
            parserStatus->state = frameCounter;
            break;
        }
        case frameCounter:
        {
            msg->header.frameCounter = data;
            parserStatus->state = reservedByte;
            break;
        }
        case reservedByte:
        {
            msg->header.reservedByte = data;
            parserStatus->state = messageLength;
            break;
        }
        case messageLength:
        {
            if (parserStatus->subState == 0)
            {
                msg->header.length = data;
                parserStatus->subState++;
            }
            else
            {
                msg->header.length |= (data << 8);
                parserStatus->state = payload;
                parserStatus->subState = 0;
            }

            break;
        }

        case payload:
        {
            if ((msg->header.length - sizeof(qHeader) > 0) && (msg->header.length < sizeof(msg->payload)))
            {
                msg->payload[parserStatus->payloadCounter++] = data;

                if (parserStatus->payloadCounter == msg->header.length - sizeof(qHeader))
                {
                    result = 1;
                    initQParser(parserStatus);
                }
            }
            else
            {
                initQParser(parserStatus);

                result = -1;
            }
            break;
        }
    }

    return result;
}
