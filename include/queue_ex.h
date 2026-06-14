#ifndef __QUEUE_EX_H__
#define __QUEUE_EX_H__

#include "os_config.h"
#include "project_def.h"

#include "queue.h"
#include "list.h"

typedef enum
{
	TRUE = 0,
	FALSE,
	Error_Par,
	Storge_Full,
	Storge_Empty,
	List_Null,
} QueueState_t;

typedef enum
{
	MessageQueue = 0,
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
 * @return
 * @note
 */
QueueState_t sQueueTake(pQueue_t this, void *Data);

/**
 * @brief  队列写缓冲区函数
 * @param  this: 队列句柄
 * @return
 * @note
 */
QueueState_t sQueueGive(pQueue_t this, void *Data);

/**
 * @brief  获取接收队列函数
 * @param  this: 队列句柄
 * @return
 * @note
 */
QueueState_t sQueueGetRxList(pQueue_t this, pList_t *pList);

/**
 * @brief  获取发送队列函数
 * @param  this: 队列句柄
 * @return
 * @note
 */
QueueState_t sQueueGetTxList(pQueue_t this, pList_t *pList);

#endif
