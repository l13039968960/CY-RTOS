#include "../../include/mutex.h"
#include "./include/queue_prv.h"

/*信号量结构体定义*/
struct Mutex
{
	Queue_t Base;

	pTCB_t HolderTask;
};

static void vMutexDestory(pQueue_t this);
static QueueState_t sMutexGive(pQueue_t this, void *Data);
static QueueState_t sMutexTake(pQueue_t this, void *Data);
static QueueState_t sMutexGetRxList(pQueue_t this, pList_t *pList);
static QueueState_t sMutexGetTxList(pQueue_t this, pList_t *pList);

static Queue_ops_t Mutex_ops = {
	.vQueueDeInit = vMutexDestory,
	.sQueueGive = sMutexGive,
	.sQueueTake = sMutexTake,
	.sQueueGetRxList = sMutexGetRxList,
	.sQueueGetTxList = sMutexGetTxList,
};

pMutex_t xMutexCreate()
{
	BaseState_t state;
	pMutex_t this;

	/*分配句柄空间*/
	state = sHeapMemAllocate(sizeof(Queue_t), (pStackType_t *)&this);
	if (state != pdTRUE)
		this = NULL;
	else
	{
		/*QueueBase初始化*/
		this->Base.Queue_Count = 1;

		this->Base.vptr = &Mutex_ops;

		sListCreatStatic(&this->Base.Queue_RxEventList);
		sListCreatStatic(&this->Base.Queue_TxEventList);

		/*MessageQueue初始化*/
		this->Base.MessageNum = 1;
		this->Base.MessageSize = 0;

		this->Base.Storage = NULL;
		this->Base.StorageTail = NULL;
		this->Base.pRead = NULL;
		this->Base.pWrite = NULL;

		this->Base.type = Mutex;

		this->HolderTask = NULL;
	}

	return this;
}

static void vMutexDestory(pQueue_t this)
{
	sHeapMemFree((pStackType_t)this);
}

static QueueState_t sMutexTake(pQueue_t this, void *Data)
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

static QueueState_t sMutexGive(pQueue_t this, void *Data)
{
	QueueState_t state;
	if (this->Queue_Count >= 1)
		state = QueueStorgeFull;
	else
	{
		this->Queue_Count++;
		state = QueueTrue;
	}

	return state;
}

static QueueState_t sMutexGetRxList(pQueue_t this, pList_t *pList)
{
	QueueState_t state;

	*pList = &this->Queue_RxEventList;
	state = QueueTrue;

	return state;
}

static QueueState_t sMutexGetTxList(pQueue_t this, pList_t *pList)
{
	QueueState_t state;

	*pList = &this->Queue_TxEventList;
	state = QueueTrue;

	return state;
}


pTCB_t xMutexGetHolder(pMutex_t this)
{
	return this->HolderTask;
}


void vMutexSetHolder(pMutex_t this, pTCB_t TCB)
{
	this->HolderTask = TCB;
}
