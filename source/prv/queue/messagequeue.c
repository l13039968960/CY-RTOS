#include "../../include/queue.h"
#include "./include/queue_prv.h"
#include "../../include/messagequeue.h"

#include <string.h>

struct MessageQueue
{
	Queue_t Base;
};

static void vMessageQueueDestory(pQueue_t this);
static QueueState_t sMessageQueueGive(pQueue_t this, void *Data);
static QueueState_t sMessageQueueTake(pQueue_t this, void *Data);
static QueueState_t sMessageQueueGetRxList(pQueue_t this, pList_t *pList);
static QueueState_t sMessageQueueGetTxList(pQueue_t this, pList_t *pList);

static Queue_ops_t MessageQueue_ops = {
	.vQueueDeInit = vMessageQueueDestory,
	.sQueueGive = sMessageQueueGive,
	.sQueueTake = sMessageQueueTake,
	.sQueueGetRxList = sMessageQueueGetRxList,
	.sQueueGetTxList = sMessageQueueGetTxList,
};

pMessageQueue_t xMessageQueueCreate(BaseType_t MessageNum, BaseType_t MessageSize)
{
	BaseState_t state;
	pMessageQueue_t this;

	/*分配句柄空间*/
	state = sHeapMemAllocate(sizeof(MessageQueue_t), (pStackType_t *)&this);
	if (state != pdTRUE)
		this = NULL;
	else
	{
		/*分配缓冲区空间*/
		state = sHeapMemAllocate(MessageNum * MessageSize, (pStackType_t *)&this->Base.Storage);
		if (state != pdTRUE)
		{
			/*失败则释放句柄空间*/
			sHeapMemFree((pStackType_t)this);
			this = NULL;
		}
		else
		{
			/*QueueBase初始化*/
			this->Base.Queue_Count = 0;

			this->Base.vptr = &MessageQueue_ops;

			sListCreatStatic(&this->Base.Queue_RxEventList);
			sListCreatStatic(&this->Base.Queue_TxEventList);

			/*MessageQueue初始化*/
			this->Base.MessageNum = MessageNum;
			this->Base.MessageSize = MessageSize;

			this->Base.StorageTail = (pQueueType_t)((uint32_t)this->Base.Storage + MessageNum * MessageSize);

			this->Base.pRead = this->Base.Storage;
			this->Base.pWrite = this->Base.Storage;

			this->Base.type = MessageQueue;
		}
	}

	return this;
}

static void vMessageQueueDestory(pQueue_t this)
{
	sHeapMemFree((pStackType_t)this->Storage);
	sHeapMemFree((pStackType_t)this);
}

static QueueState_t sMessageQueueTake(pQueue_t this, void *Data)
{
	QueueState_t state;
	if (this->Queue_Count <= 0)
		state = QueueStorgeEmpty;
	else
	{
		/*值复制*/
		memcpy(Data, (void *)this->pRead, (size_t)this->MessageSize);

		this->Queue_Count--;
		this->pRead += this->MessageSize;

		if (this->pRead >= this->StorageTail)
			this->pRead = this->Storage;

		state = QueueTrue;
	}
	return state;
}

static QueueState_t sMessageQueueGive(pQueue_t this, void *Data)
{
	QueueState_t state;
	if (this->Queue_Count >= this->MessageNum)
		state = QueueStorgeFull;
	else
	{
		/*值复制*/
		memcpy((void *)this->pWrite, Data, (size_t)this->MessageSize);

		this->Queue_Count++;
		this->pWrite += this->MessageSize;

		if (this->pWrite >= this->StorageTail)
			this->pWrite = this->Storage;

		state = QueueTrue;
	}
	return state;
}

static QueueState_t sMessageQueueGetRxList(pQueue_t this, pList_t *pList)
{
	QueueState_t state;

	*pList = &this->Queue_RxEventList;
	state = QueueTrue;

	return state;
}

static QueueState_t sMessageQueueGetTxList(pQueue_t this, pList_t *pList)
{
	QueueState_t state;

	*pList = &this->Queue_TxEventList;
	state = QueueTrue;

	return state;
}
