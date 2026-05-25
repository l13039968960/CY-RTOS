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

typedef enum
{
    Pending = 0,
    Running = 1,
} SchedulerState;

typedef pMem_Type pStack_Type;

typedef struct
{
    pStack_Type Task_TopOfStack;	/*任务栈顶*/
    pMem_Type Task_Stack;			/*任务栈指针*/

    uint8_t Task_Priority;			/*任务优先级*/
    uint32_t Task_SizeOfStack;		/*任务栈大小*/
    TaskFunction Task_Fuction;		/*任务函数*/

    List_Item StateListItem;		/*任务状态列表项*/
    List_Item EventListItem;		/*任务事件列表项*/
} TaskHandle;

typedef TaskHandle *TaskHandle_t;

/*Tick递增函数*/
rState TaskIncrementTick(void);
/*任务创建函数*/
rState TaskCreate(TaskHandle_t *taskhandle, TaskFunction Task_Fuction, uint8_t Task_Priority, uint32_t Task_SizeOfStack);
/*任务删除函数*/
rState TaskDelete(TaskHandle_t taskhandle);

void TaskStartScheduler(void);

#endif
