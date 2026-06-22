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
#include "../../include/os_config.h"
#include "heap.h"

struct TaskControBlocklHandler
{
	pStackType_t Task_TopOfStack;
	pStackType_t Task_Stack;

	uint8_t Task_Priority;

	uint32_t Task_SizeOfStack; // 栈大小

	TaskFunction Task_Fuction;

	uint8_t Task_BasePriority; // 基础优先级
	uint8_t Task_MutexCount;   // 互斥量计数器

	ListItem_t TaskStateItem;
	ListItem_t TaskEventItem;
};

typedef struct TaskControBlocklHandler TCB_t;
typedef TCB_t *pTCB_t;

/**
 * @brief  任务创建函数
 * @param  TCB: 任务句柄
 * @param  Task_Fuction: 任务函数
 * @param  Task_Priority: 任务优先级
 * @param  Task_SizeOfStack: 任务栈大小(字)
 * @return  pdTRUE: 创建成功
 *          pdFALSE: 创建失败
 * @note
 */
BaseState_t sTaskCreate(pTCB_t *TCB, TaskFunction Task_Fuction, uint8_t Task_Priority, uint32_t Task_SizeOfStack);

/**
 * @brief  任务删除函数
 * @param  taskhandle：将要删除的任务句柄
 * @return  pdTRUE: 删除成功
 *          pdFALSE: 删除失败
 * @note
 */
BaseState_t sTaskDelete(pTCB_t TCB);

#endif
