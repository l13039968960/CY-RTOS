#ifndef __OS_SEMAPHORE_H__
#define __OS_SEMAPHORE_H__

#include "os_queue.h"

#define sOSCountingSemaphopreCreate(MessageNum) ((pCountingSemaphore_t)vpOSSemaphoreCreate(MessageNum, CountingSemaphore))
#define sOSBinarySemaphoreCreate() ((pCountingSemaphore_t)vpOSSemaphoreCreate(1, BinarySemaphore))
#define sOSMutexCreate() ((pMutex_t)vpOSSemaphoreCreate(1, Mutex))

#define sOSSemaphoreDestory(SemaphoreHandler) vOSQueueDestory(SemaphoreHandler)

#define sOSSemaphoreSend(SemaphoreHandler, WaitTick) sOSQueueSend(SemaphoreHandler, NULL, WaitTick)
#define sOSSemaphoreReceive(SemaphoreHandler, WaitTick) sOSQueueReceive(SemaphoreHandler, NULL, WaitTick)

void *vpOSSemaphoreCreate(BaseType_t MessageNum, Type_t type);

/**
 * @brief  互斥量发送函数
 * @param  MutexHandler: 互斥量句柄
 * @return
 * @note
 */
OSQueueState_t sOSMutexSend(pMutex_t MutexHandler);

/**
 * @brief  互斥量发送函数
 * @param  MutexHandler: 互斥量句柄
 * @param  WaitTick: 等待时间
 * @return
 * @note
 */
OSQueueState_t sOSMutexReceive(pMutex_t MutexHandler, BaseType_t WaitTick);

#endif
