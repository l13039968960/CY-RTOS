/*************************************************************************
 * @file     : task.c
 * @brief    : 任务相关函数
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright:
 * @note     :
 *************************************************************************/

#include "task.h"
#include "heap.h"

/**
 * @brief  任务创建函数
 * @param  taskhandle: 任务句柄
 * @param  taskfuction: 任务函数
 * @param  task_priority: 任务优先级
 * @param  task_stack_size: 任务栈大小
 * @return  pdTRUE: 创建成功
 *          pdFALSE: 创建失败
 * @note
 */
state_return TaskCreate(TaskHandle_t *taskhandle, TaskFunction taskfuction, uint8_t task_priority, uint8_t task_stack_size)
{
    state_return xreturn = pdTRUE;
    memaddress stack_address, TCB;
    stack_address = MemAllocate(task_stack_size);
    if (stack_address != NULL)
    {
        TCB = MemAllocate(sizeof(TaskHandle));
        if (TCB != NULL)
        {
            *taskhandle = (TaskHandle *)TCB;
            (*taskhandle)->task_priority = task_priority;
            (*taskhandle)->task_stack = stack_address;
            (*taskhandle)->task_stack_size = task_stack_size;
            (*taskhandle)->task_state = task_ready_state;
            (*taskhandle)->taskfuction = taskfuction;
        }
        else
        {
            MemFree(stack_address);
            xreturn = pdFALSE;
        }
    }
    else
    {
        xreturn = pdFALSE;
    }
    return xreturn;
}

/**
 * @brief  任务删除函数
 * @param  taskhandle：将要删除的任务句柄
 * @return  pdTRUE: 删除成功
 *          pdFALSE: 删除失败
 * @note
 */
state_return TaskDelete(TaskHandle_t *taskhandle)
{
    state_return xreturn = pdTRUE;
    if (taskhandle != NULL)
    {
        state_return state = MemFree((*taskhandle)->task_stack);
        if (state != pdTRUE)
            xreturn = pdFALSE;
        state = MemFree(*taskhandle);
        if (state != pdTRUE)
            xreturn = pdFALSE;
    }
    else
    {
        xreturn = pdFALSE;
    }
    return xreturn;
}
