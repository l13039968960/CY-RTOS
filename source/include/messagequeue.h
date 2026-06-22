#ifndef __MESSAGEQUEUE_H__
#define __MESSAGEQUEUE_H__

#include "project_def.h"

typedef struct MessageQueue MessageQueue_t;
typedef MessageQueue_t *pMessageQueue_t;

/**
 * @brief  消息队列构造函数
 * @param  MessageNum: 消息数量
 * @param  MessageSize: 消息大小
 * @return
 * @note
 */
pMessageQueue_t xMessageQueueCreate(BaseType_t MessageNum, BaseType_t MessageSize);

#endif
