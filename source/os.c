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

pTCB_t OSCurrentTCB;						/*当前任务TCB*/
OSSchedulerState_t OSSchedulerState; /*任务调度器状态*/
pTCB_t OSNextTCB;

BaseType_t CurrentHihgestTaskPriority;
BaseType_t OSCurrentTick;	   /*当前定时器计数器*/
BaseType_t NextDelayedTaskTick; /*下一任务阻塞超时时间*/
List_t ReadyTaskList[32];	   /*就绪任务列表*/
List_t DelayTaskList;		   /*延时阻塞任务列表*/
BaseType_t OSCriticalCount;	   /*临界区计数器*/

static pTCB_t IdleTaskHandler;
static void IdleTaskFunction(void *paramter); /*空闲任务*/
static OSTaskDefType_t IdleTaskDef = {.Task_Priority = __OS_TASK_LOWEST_PRIORITY__, .Task_SizeOfStack = __OS_TASK_MINIMUN_STACKSIZE__, .Task_Fuction = IdleTaskFunction};

static pTCB_t StartTaskHandler;				   /*开始任务句柄*/
static void StartTaskFunction(void *paramter); /*开始任务*/
static OSTaskDefType_t StartTaskDef = {.Task_Priority = __OS_TASK_HIGHEST_PRIORITY__, .Task_SizeOfStack = __OS_TASK_MINIMUN_STACKSIZE__, .Task_Fuction = StartTaskFunction};

static List_t OSPendDeleteTaskList;

void vOSSwitchTCB(void);
static void vOSStartFirstTask(void);
static void vOSSwitchHighestPriority(void);

void vOSSchedulerStart(void)
{
	extern void vPortSchedluerStart(void);

	/*初始化就绪任务列表、延时阻塞任务列表*/
	sListCreatStatic(&DelayTaskList);
	for (uint8_t i = 0; i < 32; i++)
		sListCreatStatic(&ReadyTaskList[i]);

	/*初始化堆空间*/
	vHeapInit();

	/*创建空闲任务*/
	sOSTaskCreate(&IdleTaskHandler, &IdleTaskDef);
	/*创建开始任务*/
	sOSTaskCreate(&StartTaskHandler, &StartTaskDef);

	/*初始化运行变量*/
	OSCurrentTCB = NULL;
	OSSchedulerState = OSrunning;
	OSCurrentTick = 0;
	NextDelayedTaskTick = MaxDelayTime;
	OSCriticalCount = 0;

	/*切换任务TCB*/
	vOSSwitchTCB();

	/*底层配置初始化*/
	vPortSchedluerStart();

	/*开启第一个任务*/
	vOSStartFirstTask();
}

void vOSSchedulerSuspend(void)
{
}

void vOSSchedulerStop(void)
{
}

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
	if (BlockTick < NextDelayedTaskTick)
	{
		NextDelayedTaskTick = BlockTick;
	}

	/*挂起任务切换中断*/
	vOSPendSVpending();

	/*从就绪列表中移除*/
	sListItemRemove(&(OSCurrentTCB->TaskListItem), &ReadyTaskList[OSCurrentTCB->Task_Priority]);

	/*判断是否需要切换当前任务最高优先级*/
	if (OSCurrentTCB->Task_Priority == CurrentHihgestTaskPriority)
		vOSSwitchHighestPriority();

	/*添加到延时阻塞列表*/
	sListItemInsert(&(OSCurrentTCB->TaskListItem), &DelayTaskList, BlockTick);

	vOSExitCritical();
}

BaseState_t sOSTaskCreate(pTCB_t *TaskHandler, pOSTaskDefType_t TaskDefStructure)
{
	BaseState_t xstate;

	vOSEnterCritical();

	xstate = sTaskCreate(TaskHandler, TaskDefStructure->Task_Fuction, TaskDefStructure->Task_Priority, TaskDefStructure->Task_SizeOfStack);
	if (xstate != pdTRUE)
		return pdFALSE;

	xstate = sListItemInsert(&((*TaskHandler)->TaskListItem), &ReadyTaskList[(*TaskHandler)->Task_Priority], (*TaskHandler)->Task_Priority);
	if (xstate != pdTRUE)
		return pdFALSE;

		
	/*判断当前任务最高优先级是否变化*/
	if (TaskDefStructure->Task_Priority > CurrentHihgestTaskPriority)
	{
		CurrentHihgestTaskPriority = TaskDefStructure->Task_Priority;

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
			return pdFALSE;

		/*插入延迟删除列表*/
		xstate = sListItemInsert(&(TaskHandler->TaskListItem), &OSPendDeleteTaskList, 0);
		if (xstate != pdTRUE)
			return pdFALSE;
        
        if (TaskHandler->Task_Priority == CurrentHihgestTaskPriority)
			vOSSwitchHighestPriority();
		/*触发任务调度*/
		vOSPendSVpending();
	}
	else
	{
		/*从列表中移除列表项*/
		xstate = sListItemRemove(&(TaskHandler->TaskListItem), TaskHandler->TaskListItem.Container);
		if (xstate != pdTRUE)
			return pdFALSE;

		/*删除任务*/
		xstate = sTaskDelete(TaskHandler);
		if (xstate != pdTRUE)
			return pdFALSE;

		/*判断是否需要切换当前任务最高优先级*/
		if (TaskHandler->Task_Priority == CurrentHihgestTaskPriority)
			vOSSwitchHighestPriority();
	}

	vOSExitCritical();

	return pdTRUE;
}

/**
 * @brief  开启第一个任务
 * @note
 */
void vOSStartFirstTask(void)
{
	extern void vPortStartFirstTask(void);

	vPortStartFirstTask();
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
	if (OSCurrentTick >= NextDelayedTaskTick)
	{
		/*遍历所有满足条件节点*/
		while (1)
		{
			/*延时列表索引节点*/
			sListGetIndexItem(&DelayTaskList, &DelayListItem);
			if (DelayListItem == NULL)
			{
				/*首节点为空，下一个任务解释时间为最大延时时间*/
				NextDelayedTaskTick = MaxDelayTime;
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
					NextDelayedTaskTick = ItemValue;
					break;
				}
				else
				{
					/*当前计时器大于等于延时时间*/
					/*从延时列表移除*/
					sListItemRemove(DelayListItem, &DelayTaskList);

					/*插入就绪列表*/
					sListItemInsert(DelayListItem, &ReadyTaskList[TCB->Task_Priority], TCB->Task_Priority);
					if (TCB->Task_Priority > OSCurrentTCB->Task_Priority)
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
//		if (StaticTick == 10)
//		{
//			StaticTick = 0;
//			xreturn = pdTRUE;
//		}
//		else
//		{
//			xreturn = pdFALSE;
//		}
	}
	return xreturn;
}

/**
 * @brief  Systick中断
 * @note   每一Tick触发一次，判断是否需要任务切换
 */
void vPortSysTickHandler(void)
{
	// HAL_IncTick();
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
 * @brief  切换最高优先级就绪任务函数并更新索引
 * @note
 */
void vOSSwitchTCB(void)
{
	pListItem_t ListItem;
	if (OSNextTCB != NULL)
	{
		OSCurrentTCB = OSNextTCB;
		OSNextTCB = NULL;
	}
	else
	{
		sListGetIndexItem(&ReadyTaskList[CurrentHihgestTaskPriority], &ListItem);
		if (ListItem != NULL)
			OSCurrentTCB = (pTCB_t)ListItem->Owner;
	}
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

#include "../include/os_queue.h"

pQueue_t QueueTest;

 static uint32_t test1 = 0;
 void TestTask1_f(void *paramter)
 {
    uint8_t data1 = 0;
 	while (1)
 	{
 		sOSQueueReceive(QueueTest, (void *)&data1, 0xFFFFFFFF);
        test1++;
 	}
 }
 OSTaskDefType_t TestTask1 = {.Task_Fuction = TestTask1_f, .Task_Priority = 2, .Task_SizeOfStack = __OS_TASK_MINIMUN_STACKSIZE__};
 pTCB_t TestTask1Handler;

 static uint32_t test2 = 0;
 void TestTask2_f(void *paramter)
 {
 	while (1)
 	{
 		sOSQueueSend(QueueTest, (void *)&test2, 0xFFFFFFFF);
        test2++;
 	}
 }
 OSTaskDefType_t TestTask2 = {.Task_Fuction = TestTask2_f, .Task_Priority = 1, .Task_SizeOfStack = __OS_TASK_MINIMUN_STACKSIZE__};
 pTCB_t TestTask2Handler;

 
 
/**
 * @brief  开始任务函数
 * @note
 */
static void StartTaskFunction(void *paramter)
{
    QueueTest = sOSQueueCreate(1,1,MessageQueue);
	sOSTaskCreate(&TestTask1Handler, &TestTask1);
	sOSTaskCreate(&TestTask2Handler, &TestTask2);

	sOSTaskDelete(NULL);
	while (1)
	{
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
			CurrentHihgestTaskPriority = i;
			break;
		}
	}
}
