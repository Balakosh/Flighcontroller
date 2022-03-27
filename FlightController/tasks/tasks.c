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

Task_Handle heartbeatTaskHandle;
char heartbeatTaskStack[HEARTBEAT_TASK_STACKSIZE];
Task_Struct heartbeatTaskStruct;

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
}
