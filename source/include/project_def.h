/*************************************************************************
 * @file     : project_def.h
 * @brief    : 相关数据定义
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright:
 * @note     :
 *************************************************************************/
#ifndef __PROJECTDEF_H__
#define __PROJECTDEF_H__

#include "stdint.h"
#include "stdio.h"

typedef enum
{
	pdFALSE = 0,
	pdTRUE
} BaseState_t;

typedef void (*TaskFunction)(void *);
typedef uint32_t BaseType_t;

#define MaxDelayTime 0xFFFFFFFFULL

#define __is_null__(P) \
	if (P == NULL)     \
	return pdFALSE

#define __is_zero__(P) \
	if (P < 0)         \
	return pdFALSE

#endif
