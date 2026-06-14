#include "../include/queue_ex.h"
#include "../include/queue_prv.h"
#include <string.h>

static Queue_ops_t Queue_ops = {
	.vQueueDeInit = vQueueDestory,
	.sQueueGive = sQueueGive,
	.sQueueTake = sQueueTake,
	.sQueueGetRxList = sQueueGetRxList,
	.sQueueGetTxList = sQueueGetTxList};

pQueue_t xQueueCreate(BaseType_t MessageNum, BaseType_t MessageSize)
{
	BaseState_t state;
	pQueue_t this;

	/*分配句柄空间*/
	state = sHeapMemAllocate(sizeof(Queue_t), (pStackType_t *)&this);
	if (state != pdTRUE)
		this = NULL;
	else
	{
		/*分配缓冲区空间*/
		state = sHeapMemAllocate(MessageNum * MessageSize, (pStackType_t *)&this->Storage);
		if (state != pdTRUE)
		{
			/*失败则释放句柄空间*/
			sHeapMemFree((pStackType_t)this);
			this = NULL;
		}
		else
		{
			/*QueueBase初始化*/
			this->Queue_Count = 0;

			this->vptr = &Queue_ops;

			sListCreatStatic(&this->Queue_RxEventList);
			sListCreatStatic(&this->Queue_TxEventList);

			/*MessageQueue初始化*/
			this->MessageNum = MessageNum;
			this->MessageSize = MessageSize;

			this->StorageTail = (pQueueType_t)((uint32_t)this->StorageTail + MessageNum * MessageSize);

			this->pRead = this->Storage;
			this->pWrite = this->Storage;

			this->type = MessageQueue;
		}
	}
    
    return this;
}

static void vQueueDestory(pQueue_t this)
{
	sHeapMemFree((pStackType_t)this->Storage);
	sHeapMemFree((pStackType_t)this);
}

static QueueState_t sQueueTake(pQueue_t this, void *Data)
{
	QueueState_t state;
	if (this->Queue_Count <= 0)
		state = Storge_Empty;
	else
	{
		/*值复制*/
		memcpy(Data, (void *)this->pRead, (size_t)this->MessageSize);

		this->Queue_Count--;
		this->pRead += this->MessageSize;

		if (this->pRead >= this->StorageTail)
			this->pRead = this->Storage;

		state = TRUE;
	}
	return state;
}

static QueueState_t sQueueGive(pQueue_t this, void *Data)
{
	QueueState_t state;
	if (this->Queue_Count >= this->MessageNum)
		state = Storge_Full;
	else
	{
		/*值复制*/
		memcpy((void *)this->pWrite, Data, (size_t)this->MessageSize);

		this->Queue_Count++;
		this->pWrite += this->MessageSize;

		if (this->pWrite >= this->StorageTail)
			this->pWrite = this->Storage;

		state = TRUE;
	}
	return state;
}

static QueueState_t sQueueGetRxList(pQueue_t this, pList_t *pList)
{
	QueueState_t state;

	*pList = &this->Queue_RxEventList;
	state = TRUE;

	return state;
}

static QueueState_t sQueueGetTxList(pQueue_t this, pList_t *pList)
{
	QueueState_t state;

	*pList = &this->Queue_TxEventList;
	state = TRUE;

	return state;
}

// static QueueState_t sQueueGetRxListItem(pQueue_t this, pListItem_t *pListItem)
// {
// 	QueueState_t state;
// 	if (this->Queue_RxEventList.NumberOfList == 0)
// 		state = List_Null;
// 	else
// 	{
// 		*pListItem = this->Queue_RxEventList.ListEndItem.PreListItem;

// 		state = pdTRUE;
// 	}
// 	return state;
// }
