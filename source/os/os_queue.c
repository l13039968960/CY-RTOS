#include "../../include/os_queue.h"
#include "./include/os_prv.h"
#include "../include/queue.h"

void *sOSQueueCreate(BaseType_t MessageNum, BaseType_t MessageSize, Type_t type)
{
	void *QueueHandler;
	vOSEnterCritical();

	switch (type)
	{
	case MessageQueue:
		QueueHandler = (void *)xMessageQueueCreate(MessageNum, MessageSize);
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
	OSQueueState_t sreturn;

	if (QueueHandler == NULL || Data == NULL)
		sreturn = OSQueueErrorPar;

	pTCB_t UnBlockTCB;
	BaseType_t LastTick = OSCurrentTick, HasWaitTick = 0;
	pList_t pList;

	vOSEnterCritical();
	if (sQueueGive(QueueHandler, Data) == QueueTrue)
	{
		if (sQueueGetRxList(QueueHandler, &pList) == QueueTrue)
		{
			if (pList->NumberOfList != 0)
			{
				/*获取接收阻塞列表最高优先级任务*/
				UnBlockTCB = (pTCB_t)pList->ListEndItem.PreListItem->Owner;

				/*移除阻塞事件列表*/
				sListItemRemove(&UnBlockTCB->TaskEventItem, pList);
				/*移除阻塞延时列表*/
				sListItemRemove(&UnBlockTCB->TaskStateItem, &DelayTaskList);
				/*加入就绪列表*/
				sListItemInsert(&UnBlockTCB->TaskStateItem, &ReadyTaskList[UnBlockTCB->Task_Priority], UnBlockTCB->Task_Priority);

				if (UnBlockTCB->Task_Priority > OSCurrentTCB->Task_Priority)
				{
					/*任务切换*/
					OSNextTCB = UnBlockTCB;
					vOSPendSVpending();
				}
			}
		}
		vOSExitCritical();
		sreturn = OSQueuePass;
	}
	else
	{
		if (WaitTick == 0)
		{
			vOSExitCritical();
			sreturn = OSQueuePass;
		}
		else
		{
			/*阻塞*/
			if (sQueueGetTxList(QueueHandler, &pList) == QueueTrue)
			{
				/*加入发送阻塞列表*/
				sListItemInsert(&OSCurrentTCB->TaskEventItem, pList, OSCurrentTCB->Task_Priority);
				vOSExitCritical();
				while (1)
				{
					if (HasWaitTick >= WaitTick)
					{
						sreturn = OSQueueFail;
						break;
					}
					else
					{
						vOSEnterCritical();
						/*可以写数据*/
						if (sQueueGive(QueueHandler, Data) == QueueTrue)
						{
							sListItemRemove(&OSCurrentTCB->TaskEventItem, pList);
							/*是否还有等待接收任务*/
							if (sQueueGetRxList(QueueHandler, &pList) == QueueTrue)
							{
								if (pList->NumberOfList != 0)
								{
									/*获取接收阻塞列表最高优先级任务*/
									UnBlockTCB = (pTCB_t)pList->ListEndItem.PreListItem->Owner;
									/*移除阻塞事件列表*/
									sListItemRemove(&UnBlockTCB->TaskEventItem, pList);
									/*移除阻塞延时列表*/
									sListItemRemove(&UnBlockTCB->TaskStateItem, &DelayTaskList);
									/*加入就绪列表*/
									sListItemInsert(&UnBlockTCB->TaskStateItem, &ReadyTaskList[UnBlockTCB->Task_Priority], UnBlockTCB->Task_Priority);

									if (UnBlockTCB->Task_Priority > OSCurrentTCB->Task_Priority)
									{
										/*任务切换*/
										OSNextTCB = UnBlockTCB;
										vOSPendSVpending();
									}
								}
							}
							vOSExitCritical();
							sreturn = OSQueuePass;
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
	OSQueueState_t sreturn;

	if (QueueHandler == NULL)
		sreturn = OSQueueErrorPar;

	pTCB_t UnBlockTCB;
	BaseType_t LastTick = OSCurrentTick, HasWaitTick = 0;
	pList_t pList;

	vOSEnterCritical();
	if (sQueueTake(QueueHandler, Data) == QueueTrue)
	{
		if (sQueueGetTxList(QueueHandler, &pList) == QueueTrue)
		{
			if (pList->NumberOfList != 0)
			{
				/*获取发送阻塞列表最高优先级任务*/
				UnBlockTCB = (pTCB_t)pList->ListEndItem.PreListItem->Owner;

				/*移除阻塞事件列表*/
				sListItemRemove(&UnBlockTCB->TaskEventItem, pList);
				/*移除阻塞延时列表*/
				sListItemRemove(&UnBlockTCB->TaskStateItem, &DelayTaskList);
				/*加入就绪列表*/
				sListItemInsert(&UnBlockTCB->TaskStateItem, &ReadyTaskList[UnBlockTCB->Task_Priority], UnBlockTCB->Task_Priority);
			}

			if (UnBlockTCB->Task_Priority > OSCurrentTCB->Task_Priority)
			{
				/*任务切换*/
				OSNextTCB = UnBlockTCB;
				vOSPendSVpending();
			}
		}
		vOSExitCritical();
		sreturn = OSQueuePass;
	}
	else
	{
		if (WaitTick == 0)
		{
			vOSExitCritical();
			sreturn = OSQueueFail;
		}
		else
		{
			if (sQueueGetRxList(QueueHandler, &pList) == QueueTrue)
			{
				/*加入接收阻塞列表*/
				sListItemInsert(&OSCurrentTCB->TaskEventItem, pList, OSCurrentTCB->Task_Priority);
				vOSExitCritical();
				while (1)
				{
					/*阻塞时间已到*/
					if (HasWaitTick >= WaitTick)
					{
						vOSEnterCritical();
						sListItemRemove(&OSCurrentTCB->TaskEventItem, pList);

						sreturn = OSQueueFail;
						break;
					}
					else
					{
						vOSEnterCritical();
						/*可以收数据*/
						if (sQueueTake(QueueHandler, Data) == QueueTrue)
						{
							/*从接收阻塞列表移除*/
							sListItemRemove(&OSCurrentTCB->TaskEventItem, pList);

							/*是否还有等待发送任务*/
							if (sQueueGetTxList(QueueHandler, &pList) == QueueTrue)
							{
								if (pList->NumberOfList != 0)
								{
									/*获取接收阻塞列表最高优先级任务*/
									UnBlockTCB = (pTCB_t)pList->ListEndItem.PreListItem->Owner;
									/*移除阻塞事件列表*/
									sListItemRemove(&UnBlockTCB->TaskEventItem, pList);
									/*移除阻塞延时列表*/
									sListItemRemove(&UnBlockTCB->TaskStateItem, &DelayTaskList);
									/*加入就绪列表*/
									sListItemInsert(&UnBlockTCB->TaskStateItem, &ReadyTaskList[UnBlockTCB->Task_Priority], UnBlockTCB->Task_Priority);

									if (UnBlockTCB->Task_Priority > OSCurrentTCB->Task_Priority)
									{
										/*任务切换*/
										OSNextTCB = UnBlockTCB;
										vOSPendSVpending();
									}
								}
							}
							vOSExitCritical();
							sreturn = OSQueuePass;
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
