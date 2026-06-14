#ifndef __QUEUE_PRV_H__
#define __QUEUE_PRV_H__

#include "heap.h"
#include "list.h"

typedef struct Queue_ops
{
	void (*vQueueDeInit)(pQueue_t);
	QueueState_t (*sQueueTake)(pQueue_t, void *);
	QueueState_t (*sQueueGive)(pQueue_t, void *);
	QueueState_t (*sQueueGetRxList)(pQueue_t, pList_t *);
	QueueState_t (*sQueueGetTxList)(pQueue_t, pList_t *);
} Queue_ops_t;

typedef uint8_t QueueType_t;
typedef QueueType_t *pQueueType_t;

/*队列基类*/
struct Queue
{
	Queue_ops_t *vptr; /*虚函数表*/

	BaseType_t Queue_Count; /*计数器*/

	pQueueType_t Storage;	  /*消息队列缓存区*/
	pQueueType_t StorageTail; /*消息队列缓存区末尾*/

	pQueueType_t pWrite; /*写指针*/
	pQueueType_t pRead;	 /*读指针*/

	BaseType_t MessageNum;	/*消息数目*/
	BaseType_t MessageSize; /*消息大小*/

	List_t Queue_RxEventList; /*队列接收阻塞列表*/
	List_t Queue_TxEventList; /*队列发送阻塞列表*/

	Type_t type;
};

#endif
