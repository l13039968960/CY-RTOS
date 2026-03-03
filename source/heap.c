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

static const uint8_t MemBlockSize = ((sizeof(MCB) & Mem_aligen) != 0) ? ((sizeof(MCB) - (Mem_aligen & sizeof(MCB))) + 8) : sizeof(MCB);

static List FreeMemoryBlockList;

static void FreeBlockMerge(MCB *RemoveBlock);

/**
 * @brief  内存分配函数
 * @param  memsize: 内存大小
 * @return  memaddress：内存首地址
 *          NULL: 分配失败
 * @note
 */
void *MemAllocate(uint64_t memsize)
{
    MCB *NewMemBlock;
    void *xreturn = NULL;
    uint64_t wanted_memsize = memsize + MemBlockSize;
    /*确保分配的内存块为8字节对齐*/
    if ((wanted_memsize & Mem_aligen) != 0)
        wanted_memsize = (wanted_memsize - (Mem_aligen & wanted_memsize)) + 8;
    /*判断内存大小是否合理*/
    if ((wanted_memsize > 0) && (wanted_memsize < Memstack_size))
    {
        List_Item *pListItem = (List_Item *)(&(FreeMemoryBlockList.ListEndItem));
        for (pListItem = pListItem->NextListItem;
             (wanted_memsize > ((MCB *)(pListItem->Owner))->block_size) && (pListItem != (List_Item *)(&(FreeMemoryBlockList.ListEndItem)));
             pListItem = pListItem->NextListItem)
            ;
        if (pListItem == NULL)
            return NULL;
        else
        {
            NewMemBlock = (MCB *)pListItem->Owner;
            uint64_t ItemValue = NewMemBlock->block_size;
            NewMemBlock->block_size = wanted_memsize;
            // NewMemBlock->Owner = Owner;
            xreturn = (void *)(NewMemBlock + MemBlockSize);

            ListItemRemove(&(NewMemBlock->MemoryListItem));
            /*判断该内存块剩余大小可否再分为一个空闲块*/
            if (ItemValue - wanted_memsize >= MemBlockSize)
            {
                /*新MCB初始化*/
                uint64_t Newmemaddress = NewMemBlock->block_base_address + wanted_memsize;
                NewMemBlock = (MCB *)&Memstack[Newmemaddress];
                NewMemBlock->block_base_address = Newmemaddress;
                NewMemBlock->block_size = ItemValue - wanted_memsize;
                // NewMemBlock->Owner = NULL;

                /*新MCB列表项初始化*/
                NewMemBlock->MemoryListItem.Owner = (void *)NewMemBlock;
                NewMemBlock->MemoryListItem.Owner = (void *)NewMemBlock;
                ListItemInsert(&(NewMemBlock->MemoryListItem), &FreeMemoryBlockList, Newmemaddress);
            }
            else
            {
                NewMemBlock->block_size = ItemValue;
            }
        }
    }
    else
    {
        return NULL;
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
state_return MemFree(memaddress MemAddress)
{
    MCB *WillFreeBlock, *PreMemoryBlock, *NextMemoryBlock;

    WillFreeBlock = (MCB *)(MemAddress);
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
        FreeBlockMerge(PreMemoryBlock);
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
    /*初始化初始内存块*/
    MCB *InitBlock = (MCB *)&Memstack;
    InitBlock->block_base_address = 0;
    InitBlock->block_size = Memstack_size;
    // InitBlock->block_state = Free_state;
    // InitBlock->Owner = NULL;

    /*初始化初始内存块列表项*/
    InitBlock->MemoryListItem.ItemValue = 0;
    InitBlock->MemoryListItem.Container = &FreeMemoryBlockList;
    InitBlock->MemoryListItem.Owner = (void *)InitBlock;

    /*插入空闲内存块列表*/
    ListItemInsert(&(InitBlock->MemoryListItem), &FreeMemoryBlockList, 0);
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
