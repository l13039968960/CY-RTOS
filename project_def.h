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

typedef unsigned char uint8_t;
typedef char int8_t;
typedef unsigned short uint16t;
typedef short int16_t;
typedef unsigned int uint32_t;
typedef int int32_t;

typedef void (*TaskFunction)(void *);

typedef uint8_t state_return;
#define pdTRUE 1
#define pdFALSE 0

#define NULL ((void *)0)

typedef uint8_t *memaddress;

#endif