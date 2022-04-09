/*
 * twoComplement.c
 *
 *  Created on: Jun 1, 2021
 *      Author: akos
 */

#include <stdint.h>

int32_t ConvertTwosComplementShortToInteger(int16_t rawValue)
{
    // If a positive value, return it
    if ((rawValue & 0x8000) == 0)
    {
        return rawValue;
    }

    // Otherwise perform the 2's complement math on the value
    return (int16_t)(~(rawValue - 0x01)) * -1;
}
