#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "messagequeue.h"
#include "countingsemaphore.h"
#include "mutex.h"
#include "project_def.h"

typedef struct Queue Queue_t;
typedef Queue_t *pQueue_t;

typedef enum
{
	QueueTrue,
	QueueFalse,
	QueueErrorPar,
	QueueStorgeFull,
	QueueStorgeEmpty,
	QueueListNull,
} QueueState_t;

typedef enum
{
	MessageQueue,
	CountingSemaphore,
	BinarySemaphore,
	Mutex,
} Type_t;

/**
 * @brief  队列析构函数
 * @param  this: 队列句柄
 * @return
 * @note
 */
void vQueueDestory(pQueue_t this);

/**
 * @brief  队列读缓冲区函数
 * @param  this: 队列句柄
 * @param  Data: 数据指针
 * @return
 * @note
 */
QueueState_t sQueueTake(pQueue_t this, void *Data);

/**
 * @brief  队列写缓冲区函数
 * @param  this: 队列句柄
 * @param  Data: 数据指针
 * @return
 * @note
 */
QueueState_t sQueueGive(pQueue_t this, void *Data);

/**
 * @brief  获取接收队列函数
 * @param  this: 队列句柄
 * @param  pList: 列表指针
 * @return
 * @note
 */
QueueState_t sQueueGetRxList(pQueue_t this, pList_t *pList);

/**
 * @brief  获取发送队列函数
 * @param  this: 队列句柄
 * @param  pList: 列表指针
 * @return
 * @note
 */
QueueState_t sQueueGetTxList(pQueue_t this, pList_t *pList);

#endif
