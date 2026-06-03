#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "os_config.h"
#include "project_def.h"
#include "list.h"
#include "heap.h"

/*队列结构体定义*/
struct Queue
{
	pQueueType_t Queue_Storage;		/*队列缓存区*/
	pQueueType_t Queue_StorageTail; /*缓存区末尾*/

	pQueueType_t Queue_pWrite; /*队列写指针*/
	pQueueType_t Queue_pRead;  /*队列读指针*/

	BaseType_t Queue_WaitingMessage; /*队列中当前消息数量*/

	BaseType_t Queue_ItemNum;  /*队列最大消息数目*/
	BaseType_t Queue_ItemSize; /*队列消息大小*/

	List_t Queue_RxEventList; /*队列接收阻塞列表*/
	List_t Queue_TxEventList; /*队列发送阻塞列表*/
};

typedef uint8_t QueueType_t;
typedef QueueType_t *pQueueType_t;

typedef struct Queue Queue_t;
typedef Queue_t *pQueue_t;

/**
 * @brief  队列创建函数
 * @param  QueueHandler: 队列句柄指针
 * @param  QueueItemNum: 队列项目数量
 * @param  QueueItemSize: 队列项目大小
 * @return  pdTRUE: 创建成功
 *          pdFALSE: 创建失败
 * @note
 */
BaseState_t sQueueCreate(pQueue_t *QueueHandler, BaseType_t QueueItemNum, BaseType_t QueueItemSize);

/**
 * @brief  队列删除函数
 * @param  QueueHandler: 队列句柄
 * @return  pdTRUE: 删除成功
 *          pdFALSE: 删除失败
 * @note
 */
BaseState_t sQueueDelete(pQueue_t QueueHandler);

/**
 * @brief  获取队列项函数
 * @param  QueueHandler：队列句柄
 * @param  Data: 数据指针
 * @return  pdTRUE: 获取成功
 *          pdFALSE: 获取失败
 * @note
 */
BaseState_t sQueueTake(pQueue_t QueueHandler, void *Data);

/**
 * @brief  释放队列项函数
 * @param  QueueHandler：队列句柄
 * @param  Data: 数据指针
 * @return  pdTRUE: 释放成功
 *          pdFALSE: 释放失败
 * @note
 */
BaseState_t sQueueGive(pQueue_t QueueHandler, void *Data);

#endif
