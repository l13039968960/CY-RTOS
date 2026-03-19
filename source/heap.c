/*************************************************************************
 * @file     : heap.c
 * @brief    : 内存管理相关函数
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright:
 * @note     :
 *************************************************************************/
#include "../include/os_config.h"

#include "../include/heap.h"

#include "stdint.h"

static uint8_t Memstack[Memstack_size * sizeof(Mem_Type)] __attribute__((aligned(8)));

static const uint8_t MCB_SturctSize = (sizeof(MCB) + MEM_ALIGN_MASK) & ~MEM_ALIGN_MASK; /*内存控制块结构体所占用的字节数(8的倍数)*/

static List FreeMemoryBlockList;

static void FreeBlockMerge(MCB *RemoveBlock);

/**
 * @brief  内存分配函数
 * @param  memsize: 分配内存大小(字节)
 * @param  pHead: 分配的内存的首地址
 * @param  pEnd: 分配的内存的尾地址
 * @return pdTRUE:分配成功
 *         pdFALSE:分配失败
 * @note
 */
rState MemAllocate(uint64_t Memsize, pMem_Type *pHead)
{
    MCB *NewMemBlock;
    rState xreturn = pdTRUE;
    uint64_t Wanted_MemsizeBytes = Memsize + MCB_SturctSize; /*所需分配的字节数*/

    /*确保分配的内存块为8字节对齐*/
    Wanted_MemsizeBytes += MEM_ALIGN_SIZE - (Wanted_MemsizeBytes & MEM_ALIGN_MASK);

    /*判断内存大小是否合理*/
    if ((Wanted_MemsizeBytes > 0) && (Wanted_MemsizeBytes < Memstack_size * sizeof(Mem_Type)))
    {
        List_Item *pListItem = (List_Item *)(&(FreeMemoryBlockList.ListEndItem)); /*索引指针指向空闲列表哨兵节点*/

        /*地址从低到高寻找第一个满足所需容量大小的空闲内存块*/
        for (pListItem = pListItem->NextListItem;
             (Wanted_MemsizeBytes > ((MCB *)(pListItem->Owner))->block_size) && (pListItem != (List_Item *)(&(FreeMemoryBlockList.ListEndItem)));
             pListItem = pListItem->NextListItem)
            ;

        /*判断是否可以找到符合条件的空闲内存块*/
        if (pListItem != (List_Item *)(&(FreeMemoryBlockList.ListEndItem)))
        {

            NewMemBlock = (MCB *)pListItem->Owner;
            uint64_t ItemValue = NewMemBlock->block_size;
            NewMemBlock->block_size = Wanted_MemsizeBytes;
            // NewMemBlock->Owner = Owner;
            *pHead = (pMem_Type)((uint32_t)NewMemBlock + (uint32_t)MCB_SturctSize);

            /*从空闲块列表中移除*/
            ListItemRemove(&(NewMemBlock->MemoryListItem));
            /*判断该内存块剩余大小可否再分为一个空闲块*/
            if (ItemValue - Wanted_MemsizeBytes >= MCB_SturctSize)
            {
                /*新MCB初始化*/
                uint64_t Newmemaddress = NewMemBlock->block_base_address + Wanted_MemsizeBytes;
                NewMemBlock = (MCB *)&Memstack[Newmemaddress];
                NewMemBlock->block_base_address = Newmemaddress;
                NewMemBlock->block_size = ItemValue - Wanted_MemsizeBytes;
                // NewMemBlock->Owner = NULL;

                /*新MCB列表项初始化*/
                NewMemBlock->MemoryListItem.Owner = (void *)NewMemBlock;
                ListItemInsert(&(NewMemBlock->MemoryListItem), &FreeMemoryBlockList, Newmemaddress);
            }
            else
            {
                NewMemBlock->block_size = ItemValue;
            }
        }
        else
        {
            /*没有找到满足条件的空闲内存块*/
            xreturn = pdFALSE;
        }
    }
    else
    {
        /*所需内存大小不符合要求*/
        xreturn = pdFALSE;
    }
    return xreturn;
}

/**
 * @brief  内存释放函数
 * @param  MemAddress: 释放的内存首地址
 * @return  pdTRUE: 释放成功
 *          pdFALSE: 释放失败
 * @note
 */
rState MemFree(Mem_Type MemAddress)
{
    MCB *WillFreeBlock, *PreMemoryBlock, *NextMemoryBlock;

    WillFreeBlock = (MCB *)((uint32_t)MemAddress - (uint32_t)MCB_SturctSize);
    List_Item *pListItem = &(WillFreeBlock->MemoryListItem);

    ListItemInsert(pListItem, &FreeMemoryBlockList, pListItem->ItemValue);

    if (pListItem->PreListItem != (List_Item *)&(FreeMemoryBlockList.ListEndItem))
    {
        PreMemoryBlock = (MCB *)pListItem->PreListItem->Owner;
    }
    else
    {
        PreMemoryBlock = NULL;
    }

    if (pListItem->NextListItem != (List_Item *)&(FreeMemoryBlockList.ListEndItem))
    {
        NextMemoryBlock = (MCB *)pListItem->NextListItem->Owner;
    }
    else
    {
        NextMemoryBlock = NULL;
    }

    /*是否可以和前一个空闲块合并*/
    if ((PreMemoryBlock != NULL) && (PreMemoryBlock->block_base_address + PreMemoryBlock->block_size == WillFreeBlock->block_base_address))
    {
        FreeBlockMerge(WillFreeBlock);
    }
    /*是否可以和后一个空闲块合并 */
    if ((NextMemoryBlock != NULL) && (WillFreeBlock->block_base_address + WillFreeBlock->block_size == NextMemoryBlock->block_base_address))
    {
        FreeBlockMerge(NextMemoryBlock);
    }
    return pdTRUE;
}

/**
 * @brief  堆栈内存初始化函数
 * @note
 */
void Heap_Init(void)
{
    /*初始化空闲内存块列表*/
    ListCreatStatic(&FreeMemoryBlockList);

    /*初始化第一个空闲内存块*/
    MCB *FirstFreeBlock = (MCB *)&Memstack;
    FirstFreeBlock->block_base_address = 0;
    FirstFreeBlock->block_size = Memstack_size;

    /*初始化初始内存块列表项*/
    FirstFreeBlock->MemoryListItem.ItemValue = 0;
    FirstFreeBlock->MemoryListItem.Container = &FreeMemoryBlockList;
    FirstFreeBlock->MemoryListItem.Owner = (void *)FirstFreeBlock;

    /*插入空闲内存块列表*/
    ListItemInsert(&(FirstFreeBlock->MemoryListItem), &FreeMemoryBlockList, 0);
}

/**
 * @brief  空闲内存块合并函数
 * @param  RemoveBlock: 将后一个内存块合并给前一个
 * @note
 */
static void FreeBlockMerge(MCB *RemoveBlock)
{
    /*更改合并后的内存块大小*/
    ((MCB *)(((&(RemoveBlock->MemoryListItem))->PreListItem)->Owner))->block_size += RemoveBlock->block_size;
    /*删除内存块*/
    ListItemRemove(&(RemoveBlock->MemoryListItem));
}
