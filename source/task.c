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

TaskHandle_t CurrentTCB;					 /*当前任务TCB*/
SchedulerState TaskSchedulerState = Pending; /*任务调度器状态*/

static int8_t CurrentHihgestTaskPriority = 0; /*当前最高任务优先级*/
static uint64_t CurrentTick;				  /*当前定时器计数器*/
static uint64_t NextDelayedTaskTick;		  /*下一任务阻塞超时时间*/
static List ReadyTaskList[32];				  /*就绪任务列表*/
static List DelayTaskList;					  /*延时阻塞任务列表*/

static TaskHandle_t IdleTask_t;				  /*空闲任务句柄*/
static void IdleTaskFunction(void *paramter); /*空闲任务函数*/

static TaskHandle_t StartTask_t;			   /*开始任务句柄*/
static void StartTaskFunction(void *paramter); /*开始任务函数*/

static rState TaskAddToEventList(TaskHandle_t TCB, List_t EventList);
static rState TaskAddToDelayList(TaskHandle_t TCB, uint64_t DelayTime);
static rState TaskAddToReadyList(TaskHandle_t TCB);

static rState TaskRemoveFromStateList(TaskHandle_t TCB);
static rState TaskRemoveFromEventList(TaskHandle_t TCB);

static void TaskCurrentHighestTaskPriority_Switch(void);

/**
 * @brief  Tick递增函数
 * @return  pdTRUE: 需要任务切换
 *          pdFALSE: 不需要任务切换
 * @note
 */
rState TaskIncrementTick(void)
{
	/*变量定义*/
	TaskHandle_t TCB;
	uint64_t ItemValue;
	List_Item *DelayListHeadItem;
	rState xreturn = pdFALSE;
	/*计数器加一*/
	CurrentTick += 1;
	/*当前计数器大于等于下一个任务解锁时间*/
	if (CurrentTick >= NextDelayedTaskTick)
	{
		/*遍历所有满足条件节点*/
		while (1)
		{
			/*延时列表首节点*/
			DelayListHeadItem = ListGetHeadItem(&DelayTaskList);
			if (DelayListHeadItem == NULL)
			{
				/*首节点为空，下一个任务解锁时间为最大延时时间*/
				NextDelayedTaskTick = MaxDelayTime;
				break;
			}
			else
			{
				/*获取任务控制块和延时时间*/
				TCB = (TaskHandle_t)(DelayListHeadItem->Owner);
				ItemValue = ListGetItemValue(DelayListHeadItem);
				if (CurrentTick < ItemValue)
				{
					/*当前计时器小于延时时间，更新下一任务解锁时间*/
					NextDelayedTaskTick = ItemValue;
					break;
				}
				else
				{
					/*当前计时器大于等于延时时间*/
					TaskRemoveFromStateList(TCB); // 移除延时列表

					TaskAddToReadyList(TCB); // 插入就绪列表
					if (TCB->Task_Priority > CurrentTCB->Task_Priority)
					{
						CurrentHihgestTaskPriority = TCB->Task_Priority;
						/*优先级大于当前任务优先级，进行任务转换*/
						xreturn = pdTRUE;
					}
				}
			}
		}
	}
	else
	{
		xreturn = pdFALSE;
	}
	return xreturn;
}

/**
 * @brief  任务创建函数
 * @param  taskhandle: 任务句柄
 * @param  taskfuction: 任务函数
 * @param  task_priority: 任务优先级
 * @param  task_stack_size: 任务栈大小(字)
 * @return  pdTRUE: 创建成功
 *          pdFALSE: 创建失败
 * @note
 */
rState TaskCreate(TaskHandle_t *taskhandle, TaskFunction Task_Fuction, uint8_t Task_Priority, uint32_t Task_SizeOfStack)
{

	extern rState TaskStackInit(TaskHandle_t TCB);

	rState xreturn = pdTRUE;
	rState xState;
	uint32_t *Stack, *TCB;

	EnterCritical();
	/*分配栈空间*/
	xState = MemAllocate(Task_SizeOfStack * 4, (pMem_Type *)&Stack);
	if (xState != pdFALSE)
	{

		/*分配栈空间成功*/
		/*分配任务控制块空间*/
		xState = MemAllocate(sizeof(TaskHandle), (pMem_Type *)&TCB);

		if (xState != pdFALSE)
		{
			/*分配任务控制块成功*/
			*taskhandle = (TaskHandle *)TCB;

			/*任务控制块初始化*/
			(*taskhandle)->Task_Priority = Task_Priority;
			(*taskhandle)->Task_SizeOfStack = Task_SizeOfStack;
			(*taskhandle)->Task_Fuction = Task_Fuction;

			/*判断当前任务最高优先级是否变化*/
			if (Task_Priority > CurrentHihgestTaskPriority)
			{
				CurrentHihgestTaskPriority = Task_Priority;

				if (TaskSchedulerState == Running)
					PendTaskSwitch(); /*挂起任务切换中断*/
			}

			/*任务堆栈初始化*/
			(*taskhandle)->Task_Stack = (pMem_Type)Stack;
			TaskStackInit(*taskhandle);

			/*事件列表项初始化*/
			(*taskhandle)->EventListItem.Owner = (void *)(*taskhandle);
			TaskAddToEventList(*taskhandle, NULL);

			/*状态列表项初始化*/
			(*taskhandle)->StateListItem.Owner = (void *)(*taskhandle);
			TaskAddToReadyList(*taskhandle);
		}
		else
		{
			/*分配任务控制块失败，释放分配的栈空间*/
			MemFree((Mem_Type)Stack);
			xreturn = pdFALSE;
		}
	}
	else
	{
		/*分配栈空间失败*/
		xreturn = pdFALSE;
	}

	ExitCritical();
	return xreturn;
}

/**
 * @brief  任务删除函数
 * @param  taskhandle：将要删除的任务句柄
 * @return  pdTRUE: 删除成功
 *          pdFALSE: 删除失败
 * @note
 */
rState TaskDelete(TaskHandle_t TCB)
{
	/*进入临界区*/
	EnterCritical();

	rState xreturn = pdTRUE;

	/*为空即为删除当前任务*/
	if (TCB == NULL)
	{
		TCB = CurrentTCB;

		if (TaskSchedulerState == Running)
			PendTaskSwitch(); /*挂起任务切换中断*/
	}

	/*移除任务列表*/
	TaskRemoveFromStateList(TCB);

	/*如果处于事件列表则移除*/
	if (TCB->EventListItem.Container != NULL)
		TaskRemoveFromEventList(TCB);

	/*判断是否需要切换当前任务最高优先级*/
	if (TCB->Task_Priority == CurrentHihgestTaskPriority)
	{
		if (ReadyTaskList[CurrentHihgestTaskPriority].NumberOfList == 0)
		{
			TaskCurrentHighestTaskPriority_Switch();
		}
	}

	/*释放栈空间*/
	rState state = MemFree((Mem_Type)(TCB->Task_Stack));
	if (state != pdTRUE)
		xreturn = pdFALSE;

	/*释放任务控制块*/
	state = MemFree((Mem_Type)TCB);
	if (state != pdTRUE)
		xreturn = pdFALSE;

	/*退出临界区*/
	ExitCritical();

	return xreturn;
}

/**
 * @brief  任务延时函数
 * @param  DelayTime：延时时间
 * @note
 */
void TaskDelay(uint64_t DelayTime)
{
	EnterCritical();

	uint64_t BlockTick = CurrentTick + DelayTime;
	TaskHandle_t TCB = CurrentTCB;

	/*更新阻塞时间*/
	if (BlockTick < NextDelayedTaskTick)
	{
		NextDelayedTaskTick = BlockTick;
	}

	PendTaskSwitch(); /*挂起任务切换中断*/

	TaskRemoveFromStateList(TCB); //Warning ！！！！！

	/*判断是否需要切换当前任务最高优先级*/
	if (TCB->Task_Priority == CurrentHihgestTaskPriority)
	{
		if (ReadyTaskList[CurrentHihgestTaskPriority].NumberOfList == 0)
		{
			TaskCurrentHighestTaskPriority_Switch();
		}
	}

	TaskAddToDelayList(TCB, BlockTick);

	ExitCritical();
}

/**
 * @brief  切换最高优先级就绪任务控制块函数
 * @note
 */
void TaskSwitchTCB(void)
{
	CurrentTCB = (TaskHandle_t)((ReadyTaskList[CurrentHihgestTaskPriority].Itemindex)->Owner);
}

/**
 * @brief  任务错误退出函数
 * @note
 */
void TaskExitError(void)
{
	DISABLE_INTERRUPT();
	printf("ERROR!!!\r\n");

	while (1)
	{
	}
}

/**
 * @brief  空闲任务函数
 * @note
 */
static void IdleTaskFunction(void *paramter)
{
	while (1)
	{
	}
}

/**
 * @brief  开始任务函数
 * @note
 */
static void StartTaskFunction(void *paramter)
{
	while (1)
	{
	}
}

/**
 * @brief  开启任务调度器函数
 * @note
 */
void TaskStartScheduler(void)
{
	extern void StartTaskSchedluer(void);

	/*初始化就绪任务列表、延时阻塞任务列表*/
	ListCreatStatic(&DelayTaskList);
	for (uint8_t i = 0; i < 32; i++)
		ListCreatStatic(&ReadyTaskList[i]);

	/*初始化堆空间*/
	Heap_Init();

	/*创建空闲任务*/
	TaskCreate(&IdleTask_t, IdleTaskFunction, __OS_TASK_LOWEST_PRIORITY__, __OS_TASK_MINIMUN_STACKSIZE__);

	/*创建开始任务*/
	TaskCreate(&StartTask_t, StartTaskFunction, __OS_TASK_HIGHEST_PRIORITY__, __OS_TASK_MINIMUN_STACKSIZE__);

	/*初始化运行变量*/
	TaskSchedulerState = Running;
	CurrentTick = 0;
	NextDelayedTaskTick = MaxDelayTime;

	/*切换任务TCB*/
	TaskSwitchTCB();

	/*更新当前任务状态*/

	/*底层配置初始化*/
	StartTaskSchedluer();

	/*不会到达，解决警告*/
	TaskAddToDelayList(NULL, 0);
	TaskRemoveFromEventList(NULL);
}

/**
 * @brief  任务插入事件列表函数
 * @param  TCB: 任务句柄
 * @param  EventList: 事件列表
 * @return  pdTRUE: 创建成功
 *          pdFALSE: 创建失败
 * @note
 */
static rState TaskAddToEventList(TaskHandle_t TCB, List_t EventList)
{
	// /*判断参数合理性*/
	// __is_null__(TCB);

	/*初始化*/
	if (EventList == NULL)
	{
		TCB->EventListItem.Container = NULL;
		TCB->EventListItem.NextListItem = NULL;
		TCB->EventListItem.PreListItem = NULL;

		return pdTRUE;
	}
	/*插入列表*/
	if (ListItemInsert(&(TCB->EventListItem), EventList, TCB->Task_Priority) == pdTRUE)
		return pdTRUE;
	else
		return pdFALSE;
}

/**
 * @brief  任务插入延时列表函数
 * @param  TCB: 任务句柄
 * @param  DelayTime: 延时时间
 * @return  pdTRUE: 创建成功
 *          pdFALSE: 创建失败
 * @note
 */
static rState TaskAddToDelayList(TaskHandle_t TCB, uint64_t DelayTime)
{
	// /*判断参数合理性*/
	// __is_null__(TCB);
	// __is_zero__(DelayTime);

	/*插入列表*/
	if (ListItemInsert(&(TCB->StateListItem), &DelayTaskList, DelayTime) == pdTRUE)
		return pdTRUE;
	else
		return pdFALSE;
}

/**
 * @brief  任务插入就绪列表函数
 * @param  TCB: 任务句柄
 * @return  pdTRUE: 创建成功
 *          pdFALSE: 创建失败
 * @note
 */
static rState TaskAddToReadyList(TaskHandle_t TCB)
{
	// /*判断参数合理性*/
	// __is_null__(TCB);

	/*插入列表*/
	if (ListItemInsert(&(TCB->StateListItem), &ReadyTaskList[TCB->Task_Priority], TCB->Task_Priority) == pdTRUE)
		return pdTRUE;
	else
		return pdFALSE;
}

/**
 * @brief  任务删除状态列表函数
 * @param  TCB: 任务句柄
 * @return  pdTRUE: 创建成功
 *          pdFALSE: 创建失败
 * @note
 */
static rState TaskRemoveFromStateList(TaskHandle_t TCB)
{
	// /*判断参数合理性*/
	// __is_null__(TCB);

	/*删除*/
	if (ListItemRemove(&(TCB->StateListItem)) == pdTRUE)
		return pdTRUE;
	else
		return pdFALSE;
}

/**
 * @brief  任务删除时间列表函数
 * @param  TCB: 任务句柄
 * @return  pdTRUE: 创建成功
 *          pdFALSE: 创建失败
 * @note
 */
static rState TaskRemoveFromEventList(TaskHandle_t TCB)
{
	// /*判断参数合理性*/
	// __is_null__(TCB);

	/*删除*/
	if (ListItemRemove(&(TCB->EventListItem)) == pdTRUE)
		return pdTRUE;
	else
		return pdFALSE;
}

/**
 * @brief  切换当前最高优先级任务
 * @note
 */
static void TaskCurrentHighestTaskPriority_Switch(void)
{
	for (int8_t i = 31; i >= 0; i--)
	{
		if (ReadyTaskList[i].NumberOfList > 0)
		{
			CurrentHihgestTaskPriority = i;
			break;
		}
	}
}
