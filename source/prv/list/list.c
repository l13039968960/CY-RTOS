/*************************************************************************
 * @file     : list.c
 * @brief    : 列表相关函数
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright:
 * @note     :
 *************************************************************************/

#include "../../include/list.h"

BaseState_t sListCreat(pList_t *List)
{
	pList_t NewList;
	/*动态分配内存*/
	sHeapMemAllocate(sizeof(List), (pStackType_t *)&NewList);
	/*分配失败*/
	if (NewList == NULL)
		return pdFALSE;

	/*分配成功*/
	/*列表索引指向哨兵节点*/
	NewList->Itemindex = (pListItem_t) & (NewList->ListEndItem);
	/*列表成员初始为0*/
	NewList->NumberOfList = 0;
	/*确保哨兵节点始终处于列表末尾*/
	NewList->ListEndItem.ItemValue = MaxDelayTime;
	/*哨兵节点首尾相连*/
	NewList->ListEndItem.NextListItem = (pListItem_t) & (NewList->ListEndItem);
	NewList->ListEndItem.PreListItem = (pListItem_t) & (NewList->ListEndItem);
	/*赋值给列表*/
	*List = NewList;

	return pdTRUE;
}

BaseState_t sListCreatStatic(pList_t List)
{
	/*判断参数合理性*/
	__is_null__(List);

	/*列表索引指向哨兵节点*/
	List->Itemindex = (pListItem_t) & (List->ListEndItem);
	/*列表成员初始为0*/
	List->NumberOfList = 0;
	/*确保哨兵节点始终处于列表末尾*/
	List->ListEndItem.ItemValue = MaxDelayTime;
	/*哨兵节点首尾相连*/
	List->ListEndItem.NextListItem = (pListItem_t) & (List->ListEndItem);
	List->ListEndItem.PreListItem = (pListItem_t) & (List->ListEndItem);

	return pdTRUE;
}

BaseState_t sListItemInsert(pListItem_t ListItem, pList_t List, BaseType_t ItemValue)
{
	/*判断参数合理性*/
	__is_null__(ListItem);
	__is_null__(List);

	pListItem_t pListItem = (pListItem_t) & (List->ListEndItem);

	ListItem->ItemValue = ItemValue;
	ListItem->Container = List;
	List->NumberOfList++;

	/*遍历列表，寻找第一个大于ItemValue的列表项*/
	for (pListItem = pListItem->NextListItem;
		 (ItemValue >= pListItem->ItemValue) && (pListItem != (pListItem_t) & (List->ListEndItem));
		 pListItem = pListItem->NextListItem)
		;

	/*插入列表项 */
	pListItem->PreListItem->NextListItem = ListItem;
	ListItem->PreListItem = pListItem->PreListItem;
	pListItem->PreListItem = ListItem;
	ListItem->NextListItem = pListItem;

	// if (List->Itemindex == (pListItem_t)&List->ListEndItem)
	// {
	// 	List->Itemindex = List->Itemindex->NextListItem;
	// }

	return pdTRUE;
}

BaseState_t sListItemRemove(pListItem_t ListItem, pList_t List)
{
	/*判断参数合理性*/
	__is_null__(ListItem);
	__is_null__(List);

	/*不存在的操作*/
	if (ListItem->Container != List)
		return pdFALSE;

	/*从列表中删除列表项*/
	ListItem->PreListItem->NextListItem = ListItem->NextListItem;
	ListItem->NextListItem->PreListItem = ListItem->PreListItem;

	List->NumberOfList--;

	/*更改列表索引*/
	if (ListItem->Container->Itemindex == ListItem)
	{
		ListItem->Container->Itemindex = ListItem->NextListItem;
	}

	ListItem->PreListItem = NULL;
	ListItem->NextListItem = NULL;
	ListItem->Container = NULL;
	ListItem->ItemValue = 0;

	return pdTRUE;
}

BaseState_t sListGetIndexItem(pList_t List, pListItem_t* ListItem)
{
	/*判断参数合理性*/
	__is_null__(List);

	/*如果列表为空则返回NULL*/
	if (List->NumberOfList == 0)
	{
		*ListItem = NULL;
		return pdTRUE;
	}

	if (List->Itemindex == (pListItem_t)&(List->ListEndItem))
	{
		List->Itemindex = List->Itemindex->NextListItem;
	}

	/*获取列表索引项*/
	*ListItem = List->Itemindex;
	/*更新索引*/
	List->Itemindex = List->Itemindex->NextListItem;

	return pdTRUE;
}

BaseState_t sListIndexToNext(pList_t List)
{
	/*判断参数合理性*/
	__is_null__(List);

	/*如果下一列表项是哨兵节点则切换为列表的头节点*/
	if (List->Itemindex->NextListItem != (ListItem_t *)&List->ListEndItem)
	{
		List->Itemindex = List->Itemindex->NextListItem;
	}
	else
	{
		List->Itemindex = List->ListEndItem.NextListItem;
	}

	return pdTRUE;
}
