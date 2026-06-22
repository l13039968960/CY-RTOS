#include "../../include/countingsemaphore.h"
#include "./include/queue_prv.h"

/*信号量结构体定义*/
struct CountingSemaphore
{
	Queue_t Base;
};

static void vCountingSemaphoreDestory(pQueue_t this);
static QueueState_t sCountingSemaphoreGive(pQueue_t this, void *Data);
static QueueState_t sCountingSemaphoreTake(pQueue_t this, void *Data);
static QueueState_t sCountingSemaphoreGetRxList(pQueue_t this, pList_t *pList);
static QueueState_t sCountingSemaphoreGetTxList(pQueue_t this, pList_t *pList);

static Queue_ops_t Semaphore_ops = {
	.vQueueDeInit = vCountingSemaphoreDestory,
	.sQueueGive = sCountingSemaphoreGive,
	.sQueueTake = sCountingSemaphoreTake,
	.sQueueGetRxList = sCountingSemaphoreGetRxList,
	.sQueueGetTxList = sCountingSemaphoreGetTxList,
};

pCountingSemaphore_t xCountingSemaphoreCreate(BaseType_t MessageNum)
{
	BaseState_t state;
	pCountingSemaphore_t this;

	/*分配句柄空间*/
	state = sHeapMemAllocate(sizeof(Queue_t), (pStackType_t *)&this);
	if (state != pdTRUE)
		this = NULL;
	else
	{
		/*QueueBase初始化*/
		this->Base.Queue_Count = 0;

		this->Base.vptr = &Semaphore_ops;

		sListCreatStatic(&this->Base.Queue_RxEventList);
		sListCreatStatic(&this->Base.Queue_TxEventList);

		/*MessageQueue初始化*/
		this->Base.MessageNum = MessageNum;
		this->Base.MessageSize = 0;

		this->Base.Storage = NULL;
		this->Base.StorageTail = NULL;
		this->Base.pRead = NULL;
		this->Base.pWrite = NULL;

		this->Base.type = CountingSemaphore;
	}

	return this;
}

static void vCountingSemaphoreDestory(pQueue_t this)
{
	/*列表销毁*/
	sHeapMemFree((pStackType_t)this);
}

static QueueState_t sCountingSemaphoreTake(pQueue_t this, void *Data)
{
	QueueState_t state;
	if (this->Queue_Count <= 0)
		state = QueueStorgeEmpty;
	else
	{
		this->Queue_Count--;
		state = QueueTrue;
	}

	return state;
}

static QueueState_t sCountingSemaphoreGive(pQueue_t this, void *Data)
{
	QueueState_t state;
	if (this->Queue_Count >= this->MessageNum)
		state = QueueStorgeFull;
	else
	{
		this->Queue_Count++;
		state = QueueTrue;
	}

	return state;
}

static QueueState_t sCountingSemaphoreGetRxList(pQueue_t this, pList_t *pList)
{
	QueueState_t state;

	*pList = &this->Queue_RxEventList;
	state = QueueTrue;

	return state;
}

static QueueState_t sCountingSemaphoreGetTxList(pQueue_t this, pList_t *pList)
{
	QueueState_t state;

	*pList = &this->Queue_TxEventList;
	state = QueueTrue;

	return state;
}
