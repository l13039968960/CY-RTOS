/*************************************************************************
 * @file     : task.h
 * @brief    : 任务相关函数
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright:
 * @note     :
 *************************************************************************/
#ifndef __TASK_H__
#define __TASK_H__

#include "project_def.h"
#include "os_config.h"

#define task_ready_state 1
#define task_running_state 0
#define task_blocked_state -1


typedef struct
{
    TaskFunction taskfuction;
    memaddress task_stack;
    uint8_t task_state;
    uint8_t task_priority;
    uint8_t task_stack_size;
} TaskHandle;

typedef TaskHandle *TaskHandle_t;
/*任务创建函数*/
state_return TaskCreate(TaskHandle_t *taskhandle, TaskFunction taskfuction, uint8_t task_priority, uint8_t task_stack_size);

/*任务删除函数*/
state_return TaskDelete(TaskHandle_t *taskhandle);

#endif
