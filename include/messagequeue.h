#ifndef __MESSAGE_QUEUE_H__
#define __MESSAGE_QUEUE_H_

#include "queue.h"

typedef struct MessageQueue MessageQueue_t;
typedef MessageQueue_t *pMessageQueue_t;

/**
 * @brief  消息队列构造函数
 * @param
 * @return  pQueueBase_t:队列指针，NULL:创建失败
 * @note
 */
pMessageQueue_t xMessageQueueCreate(BaseType_t MessageNum, BaseType_t MessageSize);



#endif
