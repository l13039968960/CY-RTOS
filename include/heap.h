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

#define Free_state 1
#define Allocated_state 0

#define Mem_aligen 0x0007

typedef struct MemBlock
{
    uint8_t block_state;
    uint8_t block_size;
    uint8_t block_base_address;
    struct MemBlock *nextblock;
} MemBlock;

uint8_t MemBlockSize = ((sizeof(MemBlock) & Mem_aligen) != 0) ? ((sizeof(MemBlock) - (Mem_aligen & sizeof(MemBlock))) + 8) : sizeof(MemBlock);

/*内存分配*/
memaddress MemAllocate(uint8_t memsize);
/*内存释放*/
state_return MemFree(memaddress MemAddress);

#endif