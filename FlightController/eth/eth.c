/*
 * eth.c
 *
 *  Created on: May 15, 2022
 *      Author: Akeman
 */

#include <stdint.h>
#include <stdbool.h>

#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/family/arm/m3/Hwi.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#include <ti/drivers/EMAC.h>
#include <ti/drivers/emac/EMACSnow.h>

#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>

#include <driverlib/pin_map.h>
#include <driverlib/gpio.h>
#include <driverlib/flash.h>

#include "eth/tcpServer.h"

#define TCPPORT 1337

#define TCPHANDLERSTACK 2048

NIMU_DEVICE_TABLE_ENTRY NIMUDeviceTable[2] = {
    {
        /* Default: use Ethernet driver */
        .init = EMACSnow_NIMUInit
    },
    {NULL}
};

EMACSnow_Object emacObjects[1];

unsigned char macAddress[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

const EMACSnow_HWAttrs emacHWAttrs[1] = {
    {
        .baseAddr = EMAC0_BASE,
        .intNum = INT_EMAC0,
        .intPriority = (~0),
        .macAddress = macAddress
    }
};

const EMAC_Config EMAC_config[] = {
    {
        .fxnTablePtr = &EMACSnow_fxnTable,
        .object = &emacObjects[0],
        .hwAttrs = &emacHWAttrs[0]
    },
    {NULL, NULL, NULL}
};

void initEMAC(void)
{
    uint32_t ulUser0, ulUser1;

    /* Get the MAC address */
    FlashUserGet(&ulUser0, &ulUser1);
    if ((ulUser0 != 0xffffffff) && (ulUser1 != 0xffffffff))
    {
        /*
         *  Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
         *  address needed to program the hardware registers, then program the MAC
         *  address into the Ethernet Controller registers.
         */
        macAddress[0] = ((ulUser0 >>  0) & 0xff);
        macAddress[1] = ((ulUser0 >>  8) & 0xff);
        macAddress[2] = ((ulUser0 >> 16) & 0xff);
        macAddress[3] = ((ulUser1 >>  0) & 0xff);
        macAddress[4] = ((ulUser1 >>  8) & 0xff);
        macAddress[5] = ((ulUser1 >> 16) & 0xff);
    }
    else if (macAddress[0] == 0xff && macAddress[1] == 0xff &&
             macAddress[2] == 0xff && macAddress[3] == 0xff &&
             macAddress[4] == 0xff && macAddress[5] == 0xff)
    {
        //System_abort("Change the macAddress variable to match your boards MAC sticker");
    }

    GPIOPinConfigure(GPIO_PF0_EN0LED0);
    GPIOPinConfigure(GPIO_PF4_EN0LED1);
    GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);


    EMAC_init();
}

void netOpenHook(void)
{
    Task_Handle taskHandle;
    Task_Params taskParams;
    Error_Block eb;

    Error_init(&eb);

    Task_Params_init(&taskParams);
    taskParams.stackSize = TCPHANDLERSTACK;
    taskParams.priority = 1;
    taskParams.arg0 = TCPPORT;
    taskHandle = Task_create((Task_FuncPtr)tcpHandler, &taskParams, &eb);

    if (taskHandle == NULL)
    {
        System_printf("netOpenHook: Failed to create tcpHandler Task\n");
    }

    System_flush();
}
