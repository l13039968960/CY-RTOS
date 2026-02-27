/*************************************************************************
 * @file     : heap.c
 * @brief    : 内存管理相关函数
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright:
 * @note     :
 *************************************************************************/

#include "../include/heap.h"

static uint8_t Memstack[Memstack_size] __attribute__((aligned(8)));

MemBlock Head_Block;

static void Heap_Init(void);

static MemBlock *FreeBlockMerge(MemBlock *PreBlock, MemBlock *LaBlock);

/**
 * @brief  内存分配函数
 * @param  memsize: 内存大小
 * @return  memaddress：内存首地址
 *          NULL: 分配失败
 * @note
 */
memaddress MemAllocate(uint8_t memsize)
{
    MemBlock *NewMemBlock;
    uint8_t wanted_memsize = memsize + MemBlockSize;
    /*确保分配的内存块为8字节对齐*/
    if ((wanted_memsize & Mem_aligen) != 0)
        wanted_memsize = (wanted_memsize - (Mem_aligen & wanted_memsize)) + 8;
    /*判断内存空间是否充足*/
    if ((wanted_memsize > 0) && (wanted_memsize < Memstack_size))
    {
        MemBlock *FreeBlockNode = Head_Block.nextblock;
        MemBlock *FreeBlockPreNode = &Head_Block;
        /*遍历空闲内存块列表，找到地址从小到大第一个满足所需分配大小的空闲内存块 */
        for (; (FreeBlockNode->block_size < wanted_memsize) && (FreeBlockNode->nextblock != NULL);
             FreeBlockPreNode = FreeBlockNode, FreeBlockNode = FreeBlockNode->nextblock)
            ;
        if ((FreeBlockNode->nextblock == NULL) && (FreeBlockNode->block_size < wanted_memsize))
            return NULL; // 如果是最后一个内存块并且不满足所需大小，不分配
        else             // 满足大小分配
        {

            NewMemBlock = FreeBlockNode;
            /*判断该内存块剩余大小可否再分为一个空闲块*/
            if (FreeBlockNode->block_size - wanted_memsize > MemBlockSize)
            {
                /*分出第二个内存块，并插入到空闲内存块列表*/
                FreeBlockNode = (MemBlock *)&Memstack[NewMemBlock->block_base_address + wanted_memsize];
                FreeBlockNode->block_base_address = NewMemBlock->block_base_address + wanted_memsize;
                FreeBlockNode->block_size = NewMemBlock->block_size - wanted_memsize;
                FreeBlockNode->nextblock = NewMemBlock->nextblock;
                FreeBlockPreNode->nextblock = FreeBlockNode;
            }
            else
            {
                FreeBlockPreNode->nextblock = FreeBlockNode->nextblock;
            }
            NewMemBlock->block_size = wanted_memsize;
            NewMemBlock->block_state = Allocated_state;
            NewMemBlock->nextblock = NULL;
        }
    }
    else
    {
        return NULL;
    }
    return (memaddress)(NewMemBlock + MemBlockSize);
}

/**
 * @brief  内存释放函数
 * @param  MemAddress: 释放的内存首地址
 * @return  pdTRUE: 释放成功
 *          pdFALSE: 释放失败
 * @note
 */
state_return MemFree(memaddress MemAddress)
{
    if (MemAddress != NULL)
    {
        MemBlock *WillFreeBlock = (MemBlock *)(MemAddress);
        MemBlock *FreeBlockNode = Head_Block.nextblock;
        MemBlock *FreeBlockPreNode = &Head_Block;
        /*地址从小到大寻找第一个基地址在将要释放空闲块基地址后的空闲内存块*/
        for (; (FreeBlockNode->block_base_address < WillFreeBlock->block_base_address) && (FreeBlockNode->nextblock != NULL);
             FreeBlockPreNode = FreeBlockNode, FreeBlockNode = FreeBlockNode->nextblock)
            ;
        if ((FreeBlockNode->nextblock == NULL) && (FreeBlockNode->block_base_address < WillFreeBlock->block_base_address))
        {
            /*插入末尾*/
            WillFreeBlock->nextblock = FreeBlockNode->nextblock;
            FreeBlockNode->nextblock = WillFreeBlock;
            WillFreeBlock->block_state = Free_state;
            FreeBlockPreNode = FreeBlockNode;
            FreeBlockNode = NULL;
        }
        else
        {
            /*插入中间*/
            WillFreeBlock->nextblock = FreeBlockPreNode->nextblock;
            FreeBlockPreNode->nextblock = WillFreeBlock;
            WillFreeBlock->block_state = Free_state;
        }
        /*是否可以和前一个空闲块合并*/
        if (FreeBlockPreNode->block_base_address + FreeBlockPreNode->block_size == WillFreeBlock->block_base_address)
        {
            WillFreeBlock = FreeBlockMerge(FreeBlockPreNode, WillFreeBlock);
        }
        /*是否可以和后一个空闲块合并 */
        if ((FreeBlockNode != NULL) && (WillFreeBlock->block_base_address + WillFreeBlock->block_size == FreeBlockNode->block_base_address))
        {
            WillFreeBlock = FreeBlockMerge(WillFreeBlock, FreeBlockNode);
        }
    }
    else
    {
        return pdFALSE;
    }
    return pdTRUE;
}

/**
 * @brief  堆栈内存初始化函数
 * @note
 */
static void Heap_Init(void)
{
    /*8字节对齐内存管理块*/
    if ((MemBlockSize & Mem_aligen) != 0)
        MemBlockSize = (MemBlockSize - (Mem_aligen & MemBlockSize)) + 8;
    /*初始化内存块*/
    MemBlock *InitBlock = (MemBlock *)&Memstack[0];
    InitBlock->block_base_address = 0;
    InitBlock->block_size = Memstack_size;
    InitBlock->block_state = Free_state;
    InitBlock->nextblock = NULL;
    /*初始化空闲内存块头节点*/
    Head_Block.nextblock = InitBlock;
    Head_Block.block_size = 0;
    Head_Block.block_state = Free_state;
}

/**
 * @brief  空闲内存块合并函数
 * @param  PreBlock: 前一个空闲内存块
 * @param  LaBlock: 后一个空闲内存块
 * @return  MemBlock *：合并后前一个内存块
 * @note
 */
static MemBlock *FreeBlockMerge(MemBlock *PreBlock, MemBlock *LaBlock)
{
    PreBlock->block_size += LaBlock->block_size;
    PreBlock->nextblock = LaBlock->nextblock;
    return PreBlock;
}
