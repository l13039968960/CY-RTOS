#include "../../include/os_queue.h"
#include "../../include/os_semaphore.h"
#include "./include/os_prv.h"
#include "../include/queue.h"

void *vpOSSemaphoreCreate(BaseType_t MessageNum, Type_t type)
{
	void *SemphoreHandler = NULL;
	vOSEnterCritical();

	switch (type)
	{
	case CountingSemaphore:
		SemphoreHandler = (void *)xCountingSemaphoreCreate(MessageNum);
		break;
	case BinarySemaphore:
		SemphoreHandler = (void *)xCountingSemaphoreCreate(MessageNum);
		break;
	case Mutex:
		SemphoreHandler = (void *)xMutexCreate();
		break;
	default:
		break;
	}
	vOSExitCritical();

	return SemphoreHandler;
}

OSQueueState_t sOSMutexSend(pMutex_t MutexHandler)
{
	OSQueueState_t sreturn;

	if (MutexHandler == NULL)
		sreturn = OSQueueErrorPar;

	pTCB_t UnBlockTCB;
	pList_t pList;

	vOSEnterCritical();

	/*检查调用者是否是互斥量持有者*/
	if (OSCurrentTCB != xMutexGetHolder(MutexHandler))
	{
		vOSExitCritical();
		sreturn = OSQueueFail;
	}
	else
	{
		/*释放互斥量*/
		if (sQueueGive((pQueue_t)MutexHandler, NULL) == QueueTrue)
		{
			/*清除持有者*/
			vMutexSetHolder(MutexHandler, NULL);

			OSCurrentTCB->Task_MutexCount--;
			/*恢复优先级*/
			if (OSCurrentTCB->Task_MutexCount == 0)
			{
				OSCurrentTCB->Task_Priority = OSCurrentTCB->Task_BasePriority;
			}

			/*检查是否有任务在互斥量上阻塞等待*/
			if (sQueueGetRxList((pQueue_t)MutexHandler, &pList) == QueueTrue)
			{
				if (pList->NumberOfList != 0)
				{
					/*获取阻塞列表中最高优先级任务*/
					UnBlockTCB = (pTCB_t)pList->ListEndItem.PreListItem->Owner;

					/*唤醒该任务*/
					sListItemRemove(&UnBlockTCB->TaskEventItem, pList);
					sListItemRemove(&UnBlockTCB->TaskStateItem, &DelayTaskList);
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
	}
	return sreturn;
}

OSQueueState_t sOSMutexReceive(pMutex_t MutexHandler, BaseType_t WaitTick)
{
	OSQueueState_t sreturn = OSQueueFail;

	if (MutexHandler == NULL)
		sreturn = OSQueueErrorPar;

	pTCB_t HolderTCB;
	BaseType_t LastTick = OSCurrentTick, HasWaitTick = 0;
	pList_t pList;

	vOSEnterCritical();

	/*尝试获取互斥量*/
	if (sQueueTake((pQueue_t)MutexHandler, NULL) == QueueTrue)
	{
		/*互斥量可用，设置持有者*/
		vMutexSetHolder(MutexHandler, OSCurrentTCB);

		/*互斥量计数器加一*/
		OSCurrentTCB->Task_MutexCount++;
		vOSExitCritical();
		sreturn = OSQueuePass;
	}
	else
	{
		/*互斥量已被持有*/
		if (WaitTick == 0)
		{
			vOSExitCritical();
			sreturn = OSQueueFail;
		}
		else
		{
			/*获取当前持有者*/
			HolderTCB = xMutexGetHolder(MutexHandler);

			/*优先级继承：提升持有者优先级到当前任务优先级*/
			if (HolderTCB != NULL)
			{
				if (OSCurrentTCB->Task_Priority > HolderTCB->Task_Priority)
				{
					/*提升持有者优先级*/
					HolderTCB->Task_Priority = OSCurrentTCB->Task_Priority;
				}
			}

			/*将当前任务加入互斥量阻塞等待列表*/
			if (sQueueGetRxList((pQueue_t)MutexHandler, &pList) == QueueTrue)
			{
				sListItemInsert(&OSCurrentTCB->TaskEventItem, pList, OSCurrentTCB->Task_Priority);
			}
			vOSExitCritical();

			while (1)
			{
				/*阻塞时间已到*/
				if (HasWaitTick >= WaitTick)
				{
					/*从阻塞列表移除*/
					vOSEnterCritical();
					if (sQueueGetRxList((pQueue_t)MutexHandler, &pList) == QueueTrue)
					{
						sListItemRemove(&OSCurrentTCB->TaskEventItem, pList);
					}
					vOSExitCritical();
					sreturn = OSQueueFail;
					break;
				}
				else
				{
					vOSEnterCritical();
					/*再次尝试获取互斥量*/
					if (sQueueTake((pQueue_t)MutexHandler, NULL) == QueueTrue)
					{
						/*成功获取互斥量*/
						if (sQueueGetRxList((pQueue_t)MutexHandler, &pList) == QueueTrue)
						{
							sListItemRemove(&OSCurrentTCB->TaskEventItem, pList);
						}

						/*设置持有者*/
						vMutexSetHolder(MutexHandler, OSCurrentTCB);

						/*互斥量计数器加一*/
						OSCurrentTCB->Task_MutexCount++;

						vOSExitCritical();
						sreturn = OSQueuePass;
						break;
					}
					else
					{
						vOSExitCritical();
						/*继续等待*/
						HasWaitTick += (OSCurrentTick - LastTick);
						LastTick = OSCurrentTick;
						vOSDelay(WaitTick - HasWaitTick);
					}
				}
			}
		}
	}
	return sreturn;
}
