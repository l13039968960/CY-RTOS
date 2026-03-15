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
#include "heap.h"

typedef enum{
    Pending = 0,
    Running = 1,
}SchedulerState;

typedef pMem_Type pStack_Type;

typedef struct
{
    pStack_Type Task_TopOfStack;
    pMem_Type Task_Stack;

    uint8_t Task_Priority;
    uint32_t Task_SizeOfStack;
    TaskFunction Task_Fuction;

    List_Item StateListItem;
    List_Item EventListItem;
} TaskHandle;

typedef TaskHandle *TaskHandle_t;

/*Tick递增函数*/
rState TaskIncrementTick(void);
/*任务创建函数*/
rState TaskCreate(TaskHandle_t *taskhandle, TaskFunction Task_Fuction, uint8_t Task_Priority, uint32_t Task_SizeOfStack);
/*任务删除函数*/
rState TaskDelete(TaskHandle_t taskhandle);

#endif
