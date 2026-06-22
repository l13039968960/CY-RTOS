#ifndef __OS_QUEUE_H__
#define __OS_QUEUE_H__

#include "../source/include/project_def.h"
#include "../source/include/queue.h"

typedef enum
{
	OSQueuePass,
	OSQueueFail,
	OSQueueErrorPar,

} OSQueueState_t;

#define sOSMessageQueueCreate(QueueHandler, MessageNum, MessageSize) ((pMessageQueue_t)sOSQueueCreate(QueueHandler, MessageNum, MessageSize, MessageQueue))

void *sOSQueueCreate(BaseType_t MessageNum, BaseType_t MessageSize, Type_t type);
/**
 * @brief  队列销毁函数
 * @param  QueueHandler: 队列句柄
 * @return
 * @note
 */
void vOSQueueDestory(pQueue_t *QueueHandler);

/**
 * @brief  队列发送函数
 * @param  QueueHandler: 队列句柄
 * @param  Data: 数据缓冲区
 * @param  WaitTick: 等待时间
 * @return
 * @note
 */
OSQueueState_t sOSQueueSend(pQueue_t QueueHandler, void *Data, BaseType_t WaitTick);

/**
 * @brief  队列接收函数
 * @param  QueueHandler: 队列句柄
 * @param  Data: 数据缓冲区
 * @param  WaitTick: 等待时间
 * @return
 * @note
 */
OSQueueState_t sOSQueueReceive(pQueue_t QueueHandler, void *Data, BaseType_t WaitTick);

#endif
