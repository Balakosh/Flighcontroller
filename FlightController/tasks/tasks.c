/*
 * tasks.c
 *
 *  Created on: Mar 27, 2022
 *      Author: Akeman
 */

#include <assert.h>
#include <stdbool.h>

#include <ti/sysbios/knl/Task.h>

#include <xdc/std.h>
#include <xdc/runtime/Error.h>

#include "tasks/tasks.h"
#include "heartbeat/heartbeat.h"
#include "debuginterface/debuginterface.h"
#include "sensors/sensor.h"

#define HEARTBEAT_TASK_STACKSIZE 512
#define SENSOR_TASK_STACKSIZE 2048
#define DEBUG_TX_TASK_STACKSIZE 1024
#define DEBUG_RX_TASK_STACKSIZE 5120

#define HEARTBEAT_TASK_PRIORITY 1
#define DEBUG_TX_TASK_PRIORITY 2
#define DEBUG_RX_TASK_PRIORITY 2
#define SENSOR_TASK_PRIORITY 3

Task_Handle heartbeatTaskHandle;

Task_Struct heartbeatTaskStruct;
Task_Struct debugRxTaskStruct;
Task_Struct debugTxTaskStruct;
Task_Struct sensorTaskStruct;

char heartbeatTaskStack[HEARTBEAT_TASK_STACKSIZE];
char debugTxTaskStack[DEBUG_TX_TASK_STACKSIZE];
char debugRxTaskStack[DEBUG_RX_TASK_STACKSIZE];
char sensorTaskStack[SENSOR_TASK_STACKSIZE];

static void initSensorTask(void)
{
    Task_Params taskParams;
    Task_Params_init(&taskParams);

    taskParams.stack = &sensorTaskStack;
    taskParams.priority = SENSOR_TASK_PRIORITY;
    taskParams.stackSize = SENSOR_TASK_STACKSIZE;
    taskParams.instance->name = (xdc_String)"Sensor";

    Task_construct(&sensorTaskStruct, (Task_FuncPtr)sensorTaskFxn, &taskParams, NULL);
}

static void initDebugTxTask(void)
{
    Task_Params taskParams;
    Task_Params_init(&taskParams);

    taskParams.stack = &debugTxTaskStack;
    taskParams.priority = DEBUG_TX_TASK_PRIORITY;
    taskParams.stackSize = DEBUG_TX_TASK_STACKSIZE;
    taskParams.instance->name = (xdc_String)"DebugTxTask";

    Task_construct(&debugTxTaskStruct, (Task_FuncPtr)writeDebugTaskFxn, &taskParams, NULL);
}

static void initDebugRxTask(void)
{
    Task_Params taskParams;
    Task_Params_init(&taskParams);

    taskParams.stack = &debugRxTaskStack;
    taskParams.priority = DEBUG_RX_TASK_PRIORITY;
    taskParams.stackSize = DEBUG_RX_TASK_STACKSIZE;
    taskParams.instance->name = (xdc_String)"DebugRxTask";

    Task_construct(&debugRxTaskStruct, (Task_FuncPtr)readDebugTaskFxn, &taskParams, NULL);
}

static void initHeartbeatTask(void)
{
    Task_Params taskParams;

    Task_Params_init(&taskParams);

    taskParams.stack = heartbeatTaskStack;
    taskParams.priority = HEARTBEAT_TASK_PRIORITY;
    taskParams.stackSize = HEARTBEAT_TASK_STACKSIZE;
    taskParams.instance->name = (xdc_String)"Heartbeat";

    Task_construct(&heartbeatTaskStruct, (Task_FuncPtr)heartBeatTaskFxn, &taskParams, NULL);
}

void initTasks(void)
{
    initHeartbeatTask();
    initDebugTxTask();
    initDebugRxTask();
    initSensorTask();
}
