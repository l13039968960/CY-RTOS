/*************************************************************************
 * @file     : task.c
 * @brief    : 任务相关函数
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright:
 * @note     :
 *************************************************************************/

#include "../include/task.h"
#include "../include/heap.h"
#include "../include/list.h"
#include "../include/os.h"
#include "../include/os_config.h"
#include "../include/project_def.h"

#include "string.h"
#include "stdio.h"
#include "stdint.h"

#define MEM_ALIGN_SIZE 0x0008
#define MEM_ALIGN_MASK 0x0007

static void vTaskStackInit(pTCB_t TCB);

BaseState_t sTaskCreate(pTCB_t *TCB, TaskFunction Task_Fuction, uint8_t Task_Priority, uint32_t Task_SizeOfStack)
{
	extern BaseState_t TaskStackInit(pTCB_t TCB);

	// EnterCritical();
	BaseState_t xreturn = sTRUE, xstate;
	pStackType_t Stack;
	pTCB_t pTCB;

	/*分配栈空间*/
	xstate = sHeapMemAllocate(Task_SizeOfStack * 4, &Stack);
	if (xstate != sFALSE)
	{
		/*分配栈空间成功*/
		/*分配任务控制块空间*/
		xstate = sHeapMemAllocate(sizeof(TCB_t), (pStackType_t *)&pTCB);
		if (xstate != sFALSE)
		{
			/*分配任务控制块成功*/
			*TCB = pTCB;

			/*任务控制块初始化*/
			(*TCB)->Task_Priority = Task_Priority;
			(*TCB)->Task_SizeOfStack = Task_SizeOfStack;
			(*TCB)->Task_Fuction = Task_Fuction;

			// /*判断当前任务最高优先级是否变化*/
			// if (Task_Priority > CurrentHihgestTaskPriority)
			// {
			// 	CurrentHihgestTaskPriority = Task_Priority;

			// 	if (TaskSchedulerState == Running)
			// 		PendTaskSwitch();
			// }

			/*任务堆栈初始化*/
			(*TCB)->Task_Stack = Stack;
			TaskStackInit(*TCB);

			(*TCB)->TaskListItem.Container = NULL;
			(*TCB)->TaskListItem.ItemValue = 0;
			(*TCB)->TaskListItem.NextListItem = NULL;
			(*TCB)->TaskListItem.PreListItem = NULL;
			(*TCB)->TaskListItem.Owner = (void *)(*TCB);
		}
		else
		{
			/*分配任务控制块失败，释放分配的栈空间*/
			sHeapMemFree(Stack);
			xreturn = sFALSE;
		}
	}
	else
	{
		/*分配栈空间失败*/
		xreturn = sFALSE;
	}

	// ExitCritical();
	return xreturn;
}

BaseState_t sTaskDelete(pTCB_t TCB)
{
	// /*进入临界区*/
	// EnterCritical();

	BaseState_t xreturn = sTRUE;
	BaseState_t xstate;
	// /*为空即为删除当前任务*/
	// if (TCB == NULL)
	// {
	// 	TCB = CurrentTCB;

	// 	if (TaskSchedulerState == Running)
	// 		PendTaskSwitch(); /*挂起任务切换中断*/
	// }

	/*移除任务列表*/
	TaskRemoveFromStateList(TCB);
	TaskRemoveFromEventList(TCB);

	// /*判断是否需要切换当前任务最高优先级*/
	// if (TCB->Task_Priority == CurrentHihgestTaskPriority)
	// {
	// 	if (ReadyTaskList[CurrentHihgestTaskPriority].NumberOfList == 0)
	// 	{
	// 		TaskCurrentHighestTaskPriority_Switch();
	// 	}
	// }

	/*释放栈空间*/
	xstate = sHeapMemFree(TCB->Task_Stack);
	if (xstate != sTRUE)
		xreturn = sFALSE;

	/*释放任务控制块*/
	xstate = sHeapMemFree((StackType_t *)TCB);
	if (xstate != sTRUE)
		xreturn = sFALSE;

	// /*退出临界区*/
	// ExitCritical();

	return xreturn;
}

/**
 * @brief  任务栈初始化函数
 * @param  TCB: 任务句柄
 * @return  pdTRUE: 创建成功
 *          pdFALSE: 创建失败
 * @note
 */
static void vTaskStackInit(pTCB_t TCB)
{
	__is_null__(TCB);

	extern void TaskExitError(void);

	/*获取任务栈顶*/
	pStackType_t TopOfStack = (pStackType_t)((uint32_t)TCB->Task_Stack + TCB->Task_SizeOfStack * sizeof(uint32_t));

	/*满足8字节对齐*/
	TopOfStack = (pStackType_t)(((uint32_t)TopOfStack + MEM_ALIGN_MASK) & ~(MEM_ALIGN_MASK));

	/*初始化栈空间*/
	memset((void *)TCB->Task_Stack, 0xa5, TCB->Task_SizeOfStack * sizeof(uint32_t));

	/*寄存器入栈*/
	TopOfStack--;
	*TopOfStack = (StackType_t)0x01000000; // PSR
	TopOfStack--;
	*TopOfStack = (StackType_t)TCB->Task_Fuction; // PC
	TopOfStack--;
	*TopOfStack = (StackType_t)TaskExitError; // LR
	TopOfStack -= 5;					   // R12,R3,R2,R1,R0
	TopOfStack -= 8;					   // R11,R10,R9,R8,R7,R6,R5,R4

	/*更新栈顶指针*/
	TCB->Task_TopOfStack = TopOfStack;

	return sTRUE;
}
