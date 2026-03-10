/*************************************************************************
 * @file     : heap.h
 * @brief    : 内存管理相关函数
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright:
 * @note     :
 *************************************************************************/

#ifndef __HEAP_H__
#define __HEAP_H__

#include "os_config.h"
#include "project_def.h"
#include "list.h"

#define Free_state 1
#define Allocated_state 0

#define MEM_ALIGN_SIZE 0x0008
#define MEM_ALIGN_MASK 0x0007

typedef uint32_t Mem_Type;
typedef uint32_t *pMem_Type;

typedef struct MemoryControlBlock
{
    // uint8_t block_state;
    uint64_t block_size;
    uint32_t block_base_address;
    List_Item MemoryListItem;
} MCB;

typedef MCB *MCB_t;
/*内存分配*/
rState MemAllocate(uint64_t Memsize, pMem_Type *pHead);
/*内存释放*/
rState MemFree(memaddress MemAddress);
/*内存初始化*/
void Heap_Init(void);

#endif
