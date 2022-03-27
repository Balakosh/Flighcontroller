/*
 * tasks.h
 *
 *  Created on: Mar 27, 2022
 *      Author: Akeman
 */

#ifndef TASKS_TASKS_H_
#define TASKS_TASKS_H_

#include <stdbool.h>

#include <ti/sysbios/knl/Task.h>

#define HEARTBEAT_TASK_STACKSIZE 512
#define HEARTBEAT_TASK_PRIORITY 1

void initTasks(void);

#endif /* TASKS_TASKS_H_ */
