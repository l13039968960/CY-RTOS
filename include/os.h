/*************************************************************************
 * @file     : os_.h
 * @brief    : 系统调度相关函数
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright:
 * @note     :
 *************************************************************************/
#ifndef __OS_H__
#define __OS_H__

#include "os_config.h"
#include "project_def.h"
#include "task.h"
#include "heap.h"
#include "queue.h"

// #define DISABLE_INTERRUPT() RASIE_Basepri()
// #define ENABLE_INTERRUPT() Clear_Basepri()

struct OSTaskDefType
{
	uint32_t Task_Priority;

	uint32_t Task_SizeOfStack; // 字

	TaskFunction Task_Fuction;
};
typedef struct OSTaskDefType OSTaskDefType_t;
typedef OSTaskDefType_t *pOSTaskDefType_t;

struct OSQueueDefType
{
	BaseType_t Queue_DataNum;

	BaseType_t Queue_DataSize;
};

typedef struct OSQueueDefType OSQueueDefType_t;
typedef OSQueueDefType_t *pOSQueueDefType_t;

/**
 * @brief  开启任务调度器函数
 * @note
 */
void vOSSchedulerStart(void);

/**
 * @brief  挂起任务调度器
 * @note
 */
void vOSSchedulerSuspend(void);

/**
 * @brief  停止任务调度器
 * @note
 */
void vOSSchedulerStop(void);

/**
 * @brief  挂起任务调度中断
 * @note   执行一次任务切换
 */
void vOSPendSVpending(void);

/**
 * @brief  进入临界区
 * @note
 */
void vOSEnterCritical(void);

/**
 * @brief  退出临界区
 * @note
 */
void vOSExitCritical(void);

/**
 * @brief  任务延时阻塞函数
 * @param DelayTime:延时时间
 * @note
 */
void vOSDelay(BaseType_t DelayTick);

/**
 * @brief  任务创建函数
 * @param TaskHandler:任务句柄
 * @param TaskDefStructure:任务初始化结构体
 * @retval pdTRUE:创建成功
 *		   pdFALSE:创建失败
 * @note
 */
BaseState_t sOSTaskCreate(pTCB_t *TaskHandler, pOSTaskDefType_t TaskDefStructure);

/**
 * @brief  任务删除函数
 * @param TaskHandler:任务句柄
 * @retval pdTRUE:删除成功
 *		   pdFALSE:删除失败
 * @note
 */
BaseState_t sOSTaskDelete(pTCB_t TaskHandler);

/**
 * @brief  队列创建函数
 * @param QueueHandler:队列句柄
 * @param QueueDefStructre:队列初始化结构体
 * @retval pdTRUE:创建成功
 *		   pdFALSE:创建失败
 * @note
 */
BaseState_t sOSQueueCreate(pQueue_t *QueueHandler, pOSQueueDefType_t QueueDefStructre);

/**
 * @brief  队列删除函数
 * @param QueueHandler:队列句柄
 * @retval pdTRUE:删除成功
 *		   pdFALSE:删除失败
 * @note
 */
BaseState_t sOSQueueDelete(pQueue_t *QueueHandler);

#endif
