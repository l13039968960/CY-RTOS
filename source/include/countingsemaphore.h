#ifndef __COUNTINGSEMAPHORE_H__
#define __COUNTINGSEMAPHORE_H__

#include "project_def.h"

typedef struct CountingSemaphore CountingSemaphore_t;
typedef CountingSemaphore_t *pCountingSemaphore_t;

/**
 * @brief  计数型信号量构造函数
 * @param  MessageNum: 消息数目
 * @return
 * @note
 */
pCountingSemaphore_t xCountingSemaphoreCreate(BaseType_t MessageNum);

#endif
