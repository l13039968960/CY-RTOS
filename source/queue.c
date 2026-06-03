#include "../include/queue.h"
#include <string.h>

BaseState_t sQueueCreate(pQueue_t *QueueHandler, BaseType_t QueueItemNum, BaseType_t QueueItemSize)
{
	BaseState_t xstate;
	xstate = sHeapMemAllocate(sizeof(Queue_t), (pStackType_t *)QueueHandler);
	if (xstate == pdTRUE)
	{
		xstate = sHeapMemAllocate(QueueItemNum * QueueItemSize, (pStackType_t *)&((*QueueHandler)->Queue_Storage));
		if (xstate == pdTRUE)
		{
			(*QueueHandler)->Queue_StorageTail = (pQueueType_t)((uint32_t)(*QueueHandler)->Queue_Storage + (uint32_t)((*QueueHandler)->Queue_ItemNum * (*QueueHandler)->Queue_ItemSize));

			(*QueueHandler)->Queue_ItemNum = QueueItemNum;
			(*QueueHandler)->Queue_ItemSize = QueueItemSize;

			(*QueueHandler)->Queue_pWrite = (*QueueHandler)->Queue_Storage;
			(*QueueHandler)->Queue_pRead = (*QueueHandler)->Queue_Storage;

			(*QueueHandler)->Queue_WaitingMessage = 0;

			sListCreatStatic(&((*QueueHandler)->Queue_RxEventList));
			sListCreatStatic(&((*QueueHandler)->Queue_TxEventList));
		}
		else
		{
			sHeapMemFree((StackType_t *)(*QueueHandler));
			return pdFALSE;
		}
	}
	else
	{
		return pdFALSE;
	}
	return pdTRUE;
}

BaseState_t sQueueDelete(pQueue_t QueueHandler)
{
	BaseState_t xstate;
	xstate = sHeapMemFree((StackType_t *)(QueueHandler->Queue_Storage));
	if (xstate != pdTRUE)
		return pdFALSE;
	xstate = sHeapMemFree((StackType_t *)QueueHandler);
	if (xstate != pdTRUE)
		return pdFALSE;

	return pdTRUE;
}

BaseState_t sQueueTake(pQueue_t QueueHandler, void *Data)
{
	if (QueueHandler->Queue_WaitingMessage <= 0)
		return pdFALSE;

	memcpy(Data, (void *)QueueHandler->Queue_pRead, (size_t)QueueHandler->Queue_ItemSize);

	QueueHandler->Queue_WaitingMessage--;

	QueueHandler->Queue_pRead += QueueHandler->Queue_ItemSize;

	if (QueueHandler->Queue_pRead >= QueueHandler->Queue_StorageTail)
		QueueHandler->Queue_pRead == QueueHandler->Queue_Storage;

	return pdTRUE;
}

BaseState_t sQueueGive(pQueue_t QueueHandler, void *Data)
{
	if (QueueHandler->Queue_WaitingMessage >= QueueHandler->Queue_ItemNum)
		return pdFALSE;

	memcpy((void *)QueueHandler->Queue_pWrite, Data, (size_t)QueueHandler->Queue_ItemSize);
	QueueHandler->Queue_WaitingMessage++;

	QueueHandler->Queue_pWrite += QueueHandler->Queue_ItemSize;

	if (QueueHandler->Queue_pWrite >= QueueHandler->Queue_StorageTail)
		QueueHandler->Queue_pWrite == QueueHandler->Queue_Storage;

	return pdTRUE;
}
