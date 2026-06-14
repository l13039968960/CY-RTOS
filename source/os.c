/*************************************************************************
 * @file     : os_.c
 * @brief    : 系统调度相关函数
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright:
 * @note     :
 *************************************************************************/
#include "../include/os.h"

#define OSstop 0
#define OSpending 1
#define OSrunning 2

typedef uint8_t OSSchedulerState_t;

pTCB_t OSCurrentTCB;				 /*当前任务TCB*/
OSSchedulerState_t OSSchedulerState; /*任务调度器状态*/
pTCB_t OSNextTCB;

BaseType_t OSHighestPriority;
BaseType_t OSCurrentTick;	  /*当前定时器计数器*/
BaseType_t OSNextBlockedTick; /*下一任务阻塞超时时间*/
List_t ReadyTaskList[32];	  /*就绪任务列表*/
List_t DelayTaskList;		  /*延时阻塞任务列表*/
BaseType_t OSCriticalCount;	  /*临界区计数器*/

static pTCB_t IdleTaskHandler;
static void IdleTaskFunction(void *paramter); /*空闲任务*/
static OSTaskDefType_t IdleTaskDef = {.Task_Priority = __OS_TASK_LOWEST_PRIORITY__, .Task_SizeOfStack = __OS_TASK_MINIMUN_STACKSIZE__, .Task_Fuction = IdleTaskFunction};

static List_t OSPendDeleteTaskList;

void vOSSwitchTCB(void);
static void vOSSwitchHighestPriority(void);

void vOSInit(void)
{
	/*初始化堆空间*/
	vHeapInit();

	/*初始化延时阻塞任务列表*/
	sListCreatStatic(&DelayTaskList);

	/*初始化就绪任务列表*/
	for (uint8_t i = 0; i < 32; i++)
		sListCreatStatic(&ReadyTaskList[i]);

	/*初始化运行变量*/
	OSCurrentTCB = NULL;
	OSSchedulerState = OSstop;
	OSCurrentTick = 0;
	OSNextBlockedTick = MaxDelayTime;
	OSCriticalCount = 0;
}

void vOSSchedulerStart(void)
{
	extern void vPortPendSVSysTickConfig(void);
	extern void vPortStartFirstTask(void);

	/*创建空闲任务*/
	sOSTaskCreate(&IdleTaskHandler, &IdleTaskDef);

	OSSchedulerState = OSrunning;

	/*切换任务TCB*/
	vOSSwitchTCB();

	/*底层配置初始化*/
	vPortPendSVSysTickConfig();

	/*开启第一个任务*/
	vPortStartFirstTask();
}

// void vOSSchedulerSuspend(void)
// {
// }

// void vOSSchedulerStop(void)
// {
// }

void vOSPendSVpending(void)
{
	extern void vPortPendTaskSwitch(void);

	if (OSSchedulerState == OSrunning)
		vPortPendTaskSwitch();
}

void vOSEnterCritical(void)
{
	extern void vPortRASIEBasepri(void);

	vPortRASIEBasepri();

	OSCriticalCount++;
}

void vOSExitCritical(void)
{
	extern void vPortClearBasepri(void);

	OSCriticalCount--;

	if (OSCriticalCount <= 0)
	{
		vPortClearBasepri();
	}
}

void vOSDelay(BaseType_t DelayTick)
{
	BaseType_t BlockTick = OSCurrentTick + DelayTick;

	vOSEnterCritical();

	/*更新下一解锁时间*/
	if (BlockTick < OSNextBlockedTick)
		OSNextBlockedTick = BlockTick;

	/*从就绪列表中移除*/
	sListItemRemove(&(OSCurrentTCB->TaskListItem), &ReadyTaskList[OSCurrentTCB->Task_Priority]);

	/*添加到延时阻塞列表*/
	sListItemInsert(&(OSCurrentTCB->TaskListItem), &DelayTaskList, BlockTick);

	/*切换当前任务最高优先级*/
	vOSSwitchHighestPriority();
	/*挂起任务切换中断*/
	vOSPendSVpending();

	vOSExitCritical();
}

BaseState_t sOSTaskCreate(pTCB_t *TaskHandler, pOSTaskDefType_t TaskDefStructure)
{
	BaseState_t xstate;

	vOSEnterCritical();

	xstate = sTaskCreate(TaskHandler, TaskDefStructure->Task_Fuction, TaskDefStructure->Task_Priority, TaskDefStructure->Task_SizeOfStack);
	if (xstate != pdTRUE)
	{
		vOSExitCritical();
		return pdFALSE;
	}

	xstate = sListItemInsert(&((*TaskHandler)->TaskListItem), &ReadyTaskList[(*TaskHandler)->Task_Priority], (*TaskHandler)->Task_Priority);
	if (xstate != pdTRUE)
	{
		vOSExitCritical();
		return pdFALSE;
	}

	/*判断当前任务最高优先级是否变化*/
	if (TaskDefStructure->Task_Priority > OSHighestPriority)
	{
		OSHighestPriority = TaskDefStructure->Task_Priority;

		/*触发任务调度*/
		vOSPendSVpending();
	}

	vOSExitCritical();

	return pdTRUE;
}

BaseState_t sOSTaskDelete(pTCB_t TaskHandler)
{
	BaseState_t xstate;

	vOSEnterCritical();

	if (TaskHandler == NULL || TaskHandler == OSCurrentTCB)
	{
		TaskHandler = OSCurrentTCB;

		/*从列表中移除列表项*/
		xstate = sListItemRemove(&(TaskHandler->TaskListItem), TaskHandler->TaskListItem.Container);
		if (xstate != pdTRUE)
		{
			vOSExitCritical();
			return pdFALSE;
		}

		/*插入延迟删除列表*/
		xstate = sListItemInsert(&(TaskHandler->TaskListItem), &OSPendDeleteTaskList, 0);
		if (xstate != pdTRUE)
		{
			vOSExitCritical();
			return pdFALSE;
		}

		/*切换当前任务最高优先级*/
		vOSSwitchHighestPriority();
		/*触发任务调度*/
		vOSPendSVpending();
	}
	else
	{
		/*从列表中移除列表项*/
		xstate = sListItemRemove(&(TaskHandler->TaskListItem), TaskHandler->TaskListItem.Container);
		if (xstate != pdTRUE)
		{
			vOSExitCritical();
			return pdFALSE;
		}

		/*删除任务*/
		xstate = sTaskDelete(TaskHandler);
		if (xstate != pdTRUE)
		{
			vOSExitCritical();
			return pdFALSE;
		}

		/*判断是否需要切换当前任务最高优先级*/
		if (TaskHandler->Task_Priority == OSHighestPriority)
			vOSSwitchHighestPriority();
	}
	vOSExitCritical();

	return pdTRUE;
}

/**
 * @brief  Tick递增函数,判断是否需要任务切换
 * @return  pdTRUE: 需要任务切换
 *          pdFALSE: 不需要任务切换
 * @note
 */
BaseState_t sOSIncrementTick(void)
{
	/*变量定义*/
	pTCB_t TCB;
	BaseType_t ItemValue;
	pListItem_t DelayListItem;
	static BaseType_t StaticTick = 0;
	BaseState_t xreturn = pdFALSE;
	/*计数器加一*/
	OSCurrentTick += 1;
	StaticTick += 1;
	/*当前计数器大于等于下一个任务解锁时间*/
	if (OSCurrentTick >= OSNextBlockedTick)
	{
		/*遍历所有满足条件节点*/
		while (1)
		{
			/*延时列表索引节点*/
			sListGetIndexItem(&DelayTaskList, &DelayListItem);
			if (DelayListItem == NULL)
			{
				/*首节点为空，下一个任务解释时间为最大延时时间*/
				OSNextBlockedTick = MaxDelayTime;
				break;
			}
			else
			{
				/*获取任务控制块和延时时间*/
				TCB = (pTCB_t)(DelayListItem->Owner);
				ItemValue = DelayListItem->ItemValue;

				if (OSCurrentTick < ItemValue)
				{
					/*当前计时器小于延时时间，更新下一任务解锁时间*/
					OSNextBlockedTick = ItemValue;
					break;
				}
				else
				{
					/*当前计时器大于等于延时时间*/
					/*从延时列表移除*/
					sListItemRemove(DelayListItem, &DelayTaskList);

					/*插入就绪列表*/
					sListItemInsert(DelayListItem, &ReadyTaskList[TCB->Task_Priority], TCB->Task_Priority);
					if (TCB->Task_Priority > OSHighestPriority)
					{
						OSHighestPriority = TCB->Task_Priority;
						/*优先级大于当前任务优先级，进行任务转换*/
						xreturn = pdTRUE;
					}
				}
			}
		}
	}
	else
	{
		if (StaticTick == Def_Tick)
		{
			StaticTick = 0;
			xreturn = pdTRUE;
		}
		else
		{
			xreturn = pdFALSE;
		}
	}
	return xreturn;
}

/**
 * @brief  Systick中断
 * @note   每一Tick触发一次，判断是否需要任务切换
 */
void vPortSysTickHandler(void)
{
	if (sOSIncrementTick() == pdTRUE)
	{
		vOSPendSVpending();
	}
}

/**
 * @brief  任务错误退出函数
 * @note
 */
void TaskExitError(void)
{
	vOSEnterCritical();
	printf("ERROR!!!\r\n");

	while (1)
	{
	}
}

/**
 * @brief  切换最高优先级任务TCB
 * @note
 */
void vOSSwitchTCB(void)
{
	pListItem_t pListItem;
	sListGetIndexItem(&ReadyTaskList[OSHighestPriority], &pListItem);
	OSCurrentTCB = (pTCB_t)pListItem->Owner;
}

/**
 * @brief  空闲任务函数
 * @note
 */
static void IdleTaskFunction(void *paramter)
{
	sListCreatStatic(&OSPendDeleteTaskList);
	pListItem_t NextListItem;
	while (1)
	{
		sListGetIndexItem(&OSPendDeleteTaskList, &NextListItem);
		if (NextListItem != NULL)
			sOSTaskDelete((pTCB_t)(NextListItem->Owner));
	}
}

/**
 * @brief  切换系统当前任务最高优先级
 * @note
 */
static void vOSSwitchHighestPriority(void)
{
	for (int8_t i = 31; i >= 0; i--)
	{
		if (ReadyTaskList[i].NumberOfList > 0)
		{
			OSHighestPriority = i;
			break;
		}
	}
}
