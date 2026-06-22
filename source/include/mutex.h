#ifndef __MUTEX_H__
#define __MUTEX_H__

#include "task.h"
#include "project_def.h"

typedef struct Mutex Mutex_t;
typedef Mutex_t *pMutex_t;

/**
 * @brief  互斥量构造函数
 * @return
 * @note
 */
pMutex_t xMutexCreate(void);

/**
 * @brief  获取互斥量Holder
 * @param  this:互斥量句柄
 * @return
 * @note
 */
pTCB_t xMutexGetHolder(pMutex_t this);

/**
 * @brief  设置互斥量Holder
 * @param  this:互斥量句柄
 * @param  TCB:任务句柄
 * @return
 * @note
 */
void vMutexSetHolder(pMutex_t this, pTCB_t TCB);

#endif
