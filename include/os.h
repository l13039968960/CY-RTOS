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

//#define DISABLE_INTERRUPT() RASIE_Basepri()
//#define ENABLE_INTERRUPT() Clear_Basepri()

struct OSTaskDefType
{
	uint8_t Task_Priority;

	uint32_t Task_SizeOfStack; // 字

	TaskFunction Task_Fuction;
};

typedef struct OSTaskDefType OSTaskDefType_t;
typedef OSTaskDefType_t *pOSTaskDefType_t;

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
 * @param TaskHandler:任务句柄指针
 * @param TaskDefStructure:任务结构体
 * @note
 */
BaseState_t sOSTaskCreate(pTCB_t *TaskHandler, pOSTaskDefType_t TaskDefStructure);

/**
 * @brief  任务创建函数
 * @param TaskHandler:任务句柄指针
 * @param TaskDefStructure:任务结构体
 * @note
 */
BaseState_t sOSTaskDelete(pTCB_t TaskHandler);


#endif
