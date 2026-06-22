#include "../../include/queue.h"
#include "./include/queue_prv.h"
#include "../../include/list.h"

void vQueueDestory(pQueue_t this)
{
	if (this == NULL)
		return;
	this->vptr->vQueueDeInit(this);
}

QueueState_t sQueueTake(pQueue_t this, void *Data)
{
	if (this == NULL)
		return QueueErrorPar;
	return this->vptr->sQueueTake(this, Data);
}

QueueState_t sQueueGive(pQueue_t this, void *Data)
{
	if (this == NULL)
		return QueueErrorPar;
	return this->vptr->sQueueGive(this, Data);
}

QueueState_t sQueueGetRxList(pQueue_t this, pList_t *pList)
{
	if (this == NULL)
		return QueueErrorPar;
	return this->vptr->sQueueGetRxList(this, pList);
}

QueueState_t sQueueGetTxList(pQueue_t this, pList_t *pList)
{
	if (this == NULL)
		return QueueErrorPar;
	return this->vptr->sQueueGetTxList(this, pList);
}
