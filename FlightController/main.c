/*
 * main.c
 *
 *  Created on: Mar 27, 2022
 *      Author: Akeman
 */

#include <ti/sysbios/BIOS.h>

#include "init/init.h"

int main(void)
{
    init();

    BIOS_start();

    return (0);
}
