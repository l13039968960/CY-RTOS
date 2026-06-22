/*************************************************************************
 * @file     : heap.h
 * @brief    : 内存管理相关函数
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright:
 * @note     : 采用首次适用算法
 *************************************************************************/

#ifndef __HEAP_H__
#define __HEAP_H__

#include "../../include/os_config.h"
#include "project_def.h"
#include "list.h"

/**
 * @brief  内存控制块结构体
 * @note   
 */
struct HeapMemoryControlBlockHandler
{
	/*内存块大小*/
	uint32_t BlockSize;

	/*内存块基索引*/
	uint32_t BlockBaseIdx;

	/*内存块列表项*/
	ListItem_t MemoryBlockListItem;
};

typedef struct HeapMemoryControlBlockHandler MCB_t;
/**
 * @brief  栈元素类型
 * @note   
 */
typedef uint32_t StackType_t;
typedef StackType_t * pStackType_t;


/**
 * @brief  内存分配函数
 * @param  memsize: 分配内存大小(字节)
 * @param  pHead: 分配的内存的首地址指针
 * @retval pdTRUE:分配成功
 *         pdFALSE:分配失败
 * @note
 */
BaseState_t sHeapMemAllocate(uint32_t Memsize, pStackType_t *pHead);

/**
 * @brief  内存释放函数
 * @param  MemAddress: 释放的内存首地址
 * @retval  pdTRUE: 释放成功
 *          pdFALSE: 释放失败
 * @note
 */
BaseState_t sHeapMemFree(StackType_t *MemAddress);

/**
 * @brief  堆栈内存初始化函数
 * @note   在OSInit被调用
 */
void vHeapInit(void);

#endif
