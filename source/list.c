/*************************************************************************
 * @file     : list.c
 * @brief    : 列表相关函数
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright:
 * @note     :
 *************************************************************************/
#include "../include/list.h"
#include "../include/heap.h"

/**
 * @brief  列表创建函数
 * @param  List: 创建的列表
 * @return  pdTRUE: 创建成功
 *          pdFALSE: 创建失败
 * @note
 */

state_return ListCreat(List_t *List)
{
    /*判断参数是否为空*/
    __IS_NULL__(List)

    List_t NewList;
    /*动态分配内存*/
    NewList = (List_t)MemAllocate(sizeof(List));
    /*分配失败*/
    if (NewList == NULL)
        return pdFALSE;
    /*分配成功*/
    /*列表索引指向哨兵节点*/
    NewList->Itemindex = (List_Item *)&(NewList->ListEndItem);
    /*列表成员初始为0*/
    NewList->NumberOfList = 0;
    /*确保哨兵节点始终处于列表末尾*/
    NewList->ListEndItem.ItemValue = MaxDelayTime;
    /*哨兵节点首尾相连*/
    NewList->ListEndItem.NextListItem = (List_Item *)&(NewList->ListEndItem);
    NewList->ListEndItem.PreListItem = (List_Item *)&(NewList->ListEndItem);
    /*赋值给列表*/
    *List = NewList;
    return pdTRUE;
}

/**
 * @brief  列表项插入函数
 * @param  ListItem: 插入的列表项
 * @param  List: 被插入的列表
 * @param  ItemValue: 列表项的值
 * @return pdTRUE:插入成功
 *         pdFALSE:插入失败
 * @note   列表项按列表值从小到大排序
 */
state_return ListItemInsert(List_Item *ListItem, List_t List, uint64_t ItemValue)
{
    /*判断参数是否为空*/
    __IS_NULL__(ListItem)
    __IS_NULL__(List)

    List_Item *pListItem = (List_Item *)&(List->ListEndItem);
    ListItem->ItemValue = ItemValue;
    ListItem->Container = List;
    /*遍历列表，寻找第一个大于ItemValue的列表项*/
    for (pListItem = pListItem->NextListItem;
         (ItemValue >= pListItem->ItemValue) && (pListItem != (List_Item *)&(List->ListEndItem));
         pListItem = pListItem->NextListItem)
        ;
    /*插入列表项 */
    pListItem->PreListItem->NextListItem = ListItem;
    ListItem->PreListItem = pListItem->PreListItem;
    pListItem->PreListItem = ListItem;
    ListItem->NextListItem = pListItem;
    return pdTRUE;
}
