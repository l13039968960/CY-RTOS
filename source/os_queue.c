#include "../include/os_queue.h"
#include "../include/queue_ex.h"

extern pTCB_t OSCurrentTCB;		 /*当前任务TCB*/
extern List_t ReadyTaskList[32]; /*就绪任务列表*/
extern List_t DelayTaskList;	 /*延时阻塞任务列表*/
extern BaseType_t OSCurrentTick; /*当前定时器计数器*/
extern pTCB_t OSNextTCB;

pQueue_t sOSQueueCreate(BaseType_t MessageNum, BaseType_t MessageSize, Type_t type)
{
	pQueue_t QueueHandler;
	vOSEnterCritical();

	switch (type)
	{
	case MessageQueue:
		QueueHandler = xQueueCreate(MessageNum, MessageSize);
		break;
	}

	vOSExitCritical();

	return QueueHandler;
}

void vOSQueueDestory(pQueue_t *QueueHandler)
{
	vOSEnterCritical();

	vQueueDestory(*QueueHandler);

	vOSExitCritical();

	*QueueHandler = NULL;
}

OSQueueState_t sOSQueueSend(pQueue_t QueueHandler, void *Data, BaseType_t WaitTick)
{
	OSQueueState_t sreturn = Fail;

	if (QueueHandler == NULL || Data == NULL)
		sreturn = ErrorPar;

	pTCB_t UnBlockTCB;
	BaseType_t LastTick = OSCurrentTick, HasWaitTick = 0;
	pList_t pList;

	vOSEnterCritical();
	if (sQueueGive(QueueHandler, Data) == TRUE)
	{
		if (sQueueGetRxList(QueueHandler, &pList) == TRUE)
		{
			if (pList->NumberOfList != 0)
			{
				/*获取接收阻塞列表最高优先级任务*/
				UnBlockTCB = (pTCB_t)pList->ListEndItem.PreListItem->Owner;

				/*移除阻塞事件列表*/
				sListItemRemove(&UnBlockTCB->TaskEventItem, pList);
				/*移除阻塞延时列表*/
				sListItemRemove(&UnBlockTCB->TaskListItem, &DelayTaskList);
				/*加入就绪列表*/
				sListItemInsert(&UnBlockTCB->TaskListItem, &ReadyTaskList[UnBlockTCB->Task_Priority], UnBlockTCB->Task_Priority);

				if (UnBlockTCB->Task_Priority > OSCurrentTCB->Task_Priority)
				{
					/*任务切换*/
					OSNextTCB = UnBlockTCB;
					vOSPendSVpending();
				}
			}
		}
		vOSExitCritical();
		sreturn = Pass;
	}
	else
	{
		if (WaitTick == 0)
		{
			vOSExitCritical();
			sreturn = Pass;
		}
		else
		{
			/*阻塞*/
			if (sQueueGetTxList(QueueHandler, &pList) == TRUE)
			{
				/*加入发送阻塞列表*/
				sListItemInsert(&OSCurrentTCB->TaskEventItem, pList, OSCurrentTCB->Task_Priority);
				vOSExitCritical();
				while (1)
				{
					if (HasWaitTick >= WaitTick)
					{
						sreturn = Fail;
						break;
					}
					else
					{
						vOSEnterCritical();
						/*可以写数据*/
						if (sQueueGive(QueueHandler, Data) == TRUE)
						{
							sListItemRemove(&OSCurrentTCB->TaskEventItem, pList);
							/*是否还有等待接收任务*/
							if (sQueueGetRxList(QueueHandler, &pList) == TRUE)
							{
								if (pList->NumberOfList != 0)
								{
									/*获取接收阻塞列表最高优先级任务*/
									UnBlockTCB = (pTCB_t)pList->ListEndItem.PreListItem->Owner;
									/*移除阻塞事件列表*/
									sListItemRemove(&UnBlockTCB->TaskEventItem, pList);
									/*移除阻塞延时列表*/
									sListItemRemove(&UnBlockTCB->TaskListItem, &DelayTaskList);
									/*加入就绪列表*/
									sListItemInsert(&UnBlockTCB->TaskListItem, &ReadyTaskList[UnBlockTCB->Task_Priority], UnBlockTCB->Task_Priority);

									if (UnBlockTCB->Task_Priority > OSCurrentTCB->Task_Priority)
									{
										/*任务切换*/
										OSNextTCB = UnBlockTCB;
										vOSPendSVpending();
									}
								}
							}
							vOSExitCritical();
							sreturn = Pass;
							break;
						}
						else
						{
							vOSExitCritical();
							// /*继续阻塞等待*/
							HasWaitTick += (OSCurrentTick - LastTick);
							LastTick = OSCurrentTick;
							vOSDelay(WaitTick - HasWaitTick);
						}
					}
				}
			}
		}
	}
	return sreturn;
}

OSQueueState_t sOSQueueReceive(pQueue_t QueueHandler, void *Data, BaseType_t WaitTick)
{
	OSQueueState_t sreturn = Fail;

	if (QueueHandler == NULL)
		sreturn = ErrorPar;

	pTCB_t UnBlockTCB;
	BaseType_t LastTick = OSCurrentTick, HasWaitTick = 0;
	pList_t pList;

	vOSEnterCritical();
	if (sQueueTake(QueueHandler, Data) == TRUE)
	{
		if (sQueueGetTxList(QueueHandler, &pList) == TRUE)
		{
			if (pList->NumberOfList != 0)
			{
				/*获取发送阻塞列表最高优先级任务*/
				UnBlockTCB = (pTCB_t)pList->ListEndItem.PreListItem->Owner;

				/*移除阻塞事件列表*/
				sListItemRemove(&UnBlockTCB->TaskEventItem, pList);
				/*移除阻塞延时列表*/
				sListItemRemove(&UnBlockTCB->TaskListItem, &DelayTaskList);
				/*加入就绪列表*/
				sListItemInsert(&UnBlockTCB->TaskListItem, &ReadyTaskList[UnBlockTCB->Task_Priority], UnBlockTCB->Task_Priority);
			}

			if (UnBlockTCB->Task_Priority > OSCurrentTCB->Task_Priority)
			{
				/*任务切换*/
				OSNextTCB = UnBlockTCB;
				vOSPendSVpending();
			}
		}
		vOSExitCritical();
		sreturn = Pass;
	}
	else
	{
		if (WaitTick == 0)
		{
			vOSExitCritical();
			sreturn = Fail;
		}
		else
		{
			if (sQueueGetRxList(QueueHandler, &pList) == TRUE)
			{
				/*加入接收阻塞列表*/
				sListItemInsert(&OSCurrentTCB->TaskEventItem, pList, OSCurrentTCB->Task_Priority);
				vOSExitCritical();
				while (1)
				{
					/*阻塞时间已到*/
					if (HasWaitTick >= WaitTick)
					{
						sreturn = Fail;
						break;
					}
					else
					{
						vOSEnterCritical();
						/*可以收数据*/
						if (sQueueTake(QueueHandler, Data) == TRUE)
						{
							/*从接收阻塞列表移除*/
							sListItemRemove(&OSCurrentTCB->TaskEventItem, pList);

							/*是否还有等待发送任务*/
							if (sQueueGetTxList(QueueHandler, &pList) == TRUE)
							{
								if (pList->NumberOfList != 0)
								{
									/*获取接收阻塞列表最高优先级任务*/
									UnBlockTCB = (pTCB_t)pList->ListEndItem.PreListItem->Owner;
									/*移除阻塞事件列表*/
									sListItemRemove(&UnBlockTCB->TaskEventItem, pList);
									/*移除阻塞延时列表*/
									sListItemRemove(&UnBlockTCB->TaskListItem, &DelayTaskList);
									/*加入就绪列表*/
									sListItemInsert(&UnBlockTCB->TaskListItem, &ReadyTaskList[UnBlockTCB->Task_Priority], UnBlockTCB->Task_Priority);

									if (UnBlockTCB->Task_Priority > OSCurrentTCB->Task_Priority)
									{
										/*任务切换*/
										OSNextTCB = UnBlockTCB;
										vOSPendSVpending();
									}
								}
							}
							vOSExitCritical();
							sreturn = Pass;
							break;
						}
						else
						{
							vOSExitCritical();
							// /*继续阻塞等待*/
							HasWaitTick += (OSCurrentTick - LastTick);
							LastTick = OSCurrentTick;
							vOSDelay(WaitTick - HasWaitTick);
						}
					}
				}
			}
		}
	}
	return sreturn;
}
