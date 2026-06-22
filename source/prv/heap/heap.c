/*************************************************************************
 * @file     : heap.c
 * @brief    : 内存管理相关函数
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright:
 * @note     :
 *************************************************************************/
#include "../../include/heap.h"

#define MEM_ALIGN_SIZE 0x0008
#define MEM_ALIGN_MASK 0x0007

/*内存控制块结构体所占用的字节数(8的倍数)*/
static const uint8_t MCB_SturctSize = (sizeof(MCB_t) + MEM_ALIGN_MASK) & ~MEM_ALIGN_MASK;
/*静态栈空间*/
uint8_t HeapMemstack[Memstack_size * sizeof(StackType_t)] __attribute__((aligned(8)));
/*空闲内存块列表*/
static List_t FreeMemoryBlockList;

static void vHeapBlockMerge(MCB_t *RemoveBlock);

BaseState_t sHeapMemAllocate(uint32_t WantedBytes, pStackType_t *pHead)
{
	BaseState_t xreturn = pdTRUE;
	MCB_t *NewMemBlock;
	/*所需分配的字节数*/
	WantedBytes += MCB_SturctSize;
	/*确保8字节对齐*/
	WantedBytes += MEM_ALIGN_SIZE - (WantedBytes & MEM_ALIGN_MASK);
	/*判断内存大小是否合理*/
	if ((WantedBytes > 0) && (WantedBytes < Memstack_size * sizeof(StackType_t)))
	{
		/*索引指针指向空闲列表哨兵节点*/
		pListItem_t pListItem = (pListItem_t)(&(FreeMemoryBlockList.ListEndItem));
		/*地址从低到高寻找第一个满足所需容量大小的空闲内存块*/
		for (pListItem = pListItem->NextListItem;
			 (WantedBytes > ((MCB_t *)(pListItem->Owner))->BlockSize) && (pListItem != (pListItem_t)(&(FreeMemoryBlockList.ListEndItem)));
			 pListItem = pListItem->NextListItem)
			;
		/*判断是否可以找到符合条件的空闲内存块*/
		if (pListItem != (pListItem_t)(&(FreeMemoryBlockList.ListEndItem)))
		{
			NewMemBlock = (MCB_t *)pListItem->Owner;
			uint32_t ItemValue = NewMemBlock->BlockSize;
			NewMemBlock->BlockSize = WantedBytes;
			*pHead = (pStackType_t)((uint32_t)NewMemBlock + (uint32_t)MCB_SturctSize);
			/*从空闲块列表中移除*/
			sListItemRemove(&(NewMemBlock->MemoryBlockListItem), &FreeMemoryBlockList);
			/*判断该内存块剩余大小可否再分为一个空闲块*/
			if (ItemValue - WantedBytes >= MCB_SturctSize)
			{
				/*新MCB初始化*/
				uint32_t NewMemAddr = NewMemBlock->BlockBaseIdx + WantedBytes;
				NewMemBlock = (MCB_t *)&HeapMemstack[NewMemAddr];
				NewMemBlock->BlockBaseIdx = NewMemAddr;
				NewMemBlock->BlockSize = ItemValue - WantedBytes;
				/*新MCB列表项初始化*/
				NewMemBlock->MemoryBlockListItem.Owner = (void *)NewMemBlock;
				sListItemInsert(&(NewMemBlock->MemoryBlockListItem), &FreeMemoryBlockList, NewMemAddr);
			}
			else
			{
				NewMemBlock->BlockSize = ItemValue;
			}
		}
		else
		{
			/*没有找到满足条件的空闲内存块*/
			xreturn = pdFALSE;
			*pHead = NULL;
		}
	}
	else
	{
		/*所需内存大小不符合要求*/
		xreturn = pdFALSE;
		*pHead = NULL;
	}
	return xreturn;
}

BaseState_t sHeapMemFree(StackType_t *MemAddress)
{
	MCB_t *WillFreeBlock, *PreMemoryBlock, *NextMemoryBlock;

	WillFreeBlock = (MCB_t *)((uint32_t)MemAddress - (uint32_t)MCB_SturctSize);
	pListItem_t pListItem = &(WillFreeBlock->MemoryBlockListItem);
	/*插入到空闲内存块列表*/
	sListItemInsert(pListItem, &FreeMemoryBlockList, pListItem->ItemValue);

	/*获取前一空闲块*/
	PreMemoryBlock = pListItem->PreListItem != (pListItem_t)&(FreeMemoryBlockList.ListEndItem) ? (MCB_t *)pListItem->PreListItem->Owner : NULL;
	/*获取后一空闲块*/
	NextMemoryBlock = pListItem->NextListItem != (pListItem_t)&(FreeMemoryBlockList.ListEndItem) ? (MCB_t *)pListItem->NextListItem->Owner : NULL;

	/*是否可以和前一个空闲块合并*/
	if ((PreMemoryBlock != NULL) && (PreMemoryBlock->BlockBaseIdx + PreMemoryBlock->BlockSize == WillFreeBlock->BlockBaseIdx))
		vHeapBlockMerge(WillFreeBlock);
	/*是否可以和后一个空闲块合并 */
	if ((NextMemoryBlock != NULL) && (WillFreeBlock->BlockBaseIdx + WillFreeBlock->BlockSize == NextMemoryBlock->BlockBaseIdx))
		vHeapBlockMerge(NextMemoryBlock);
		
	return pdTRUE;
}

void vHeapInit(void)
{
	/*初始化空闲内存块列表*/
	sListCreatStatic(&FreeMemoryBlockList);

	/*初始化第一个空闲内存块*/
	MCB_t *FirstFreeBlock = (MCB_t *)&HeapMemstack;
	FirstFreeBlock->BlockBaseIdx = 0;
	FirstFreeBlock->BlockSize = Memstack_size;

	/*初始化初始内存块列表项*/
	FirstFreeBlock->MemoryBlockListItem.ItemValue = 0;
	FirstFreeBlock->MemoryBlockListItem.Container = &FreeMemoryBlockList;
	FirstFreeBlock->MemoryBlockListItem.Owner = (void *)FirstFreeBlock;

	/*插入空闲内存块列表*/
	sListItemInsert(&(FirstFreeBlock->MemoryBlockListItem), &FreeMemoryBlockList, 0);
}

/**
 * @brief  空闲内存块合并函数
 * @param  RemoveBlock: 要删除的内存块
 * @note   将后一个内存块合并给前一个
 */
static void vHeapBlockMerge(MCB_t *RemoveBlock)
{
	/*更改合并后的内存块大小*/
	((MCB_t *)(((&(RemoveBlock->MemoryBlockListItem))->PreListItem)->Owner))->BlockSize += RemoveBlock->BlockSize;
	/*删除内存块*/
	sListItemRemove(&(RemoveBlock->MemoryBlockListItem), &FreeMemoryBlockList);
}

