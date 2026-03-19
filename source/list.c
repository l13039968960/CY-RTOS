/*************************************************************************
 * @file     : list.c
 * @brief    : 列表相关函数
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright:
 * @note     :
 *************************************************************************/
#include "../include/project_def.h"
#include "../include/list.h"
#include "../include/heap.h"

#include "stdint.h"

/**
 * @brief  列表创建函数
 * @param  List: 创建的列表
 * @return  rState
 * @note
 */

rState ListCreat(List_t *List)
{
    // /*判断参数合理性*/
    // __is_null__(List);

    List_t NewList;
    /*动态分配内存*/
    MemAllocate(sizeof(List), (pMem_Type *)&NewList);
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
 * @brief  静态列表创建函数
 * @param  List: 创建的列表
 * @return  rState
 * @note
 */

rState ListCreatStatic(List_t List)
{
    // /*判断参数合理性*/
    // __is_null__(List);

    /*列表索引指向哨兵节点*/
    List->Itemindex = (List_Item *)&(List->ListEndItem);
    /*列表成员初始为0*/
    List->NumberOfList = 0;
    /*确保哨兵节点始终处于列表末尾*/
    List->ListEndItem.ItemValue = MaxDelayTime;
    /*哨兵节点首尾相连*/
    List->ListEndItem.NextListItem = (List_Item *)&(List->ListEndItem);
    List->ListEndItem.PreListItem = (List_Item *)&(List->ListEndItem);

    return pdTRUE;
}

/**
 * @brief  列表项插入函数
 * @param  ListItem: 插入的列表项
 * @param  List: 被插入的列表
 * @param  ItemValue: 列表项的值
 * @return rState
 * @note
 */
rState ListItemInsert(List_Item *ListItem, List_t List, uint64_t ItemValue)
{
    // /*判断参数合理性*/
    // __is_null__(ListItem);
    // __is_null__(List);

    List_Item *pListItem = (List_Item *)&(List->ListEndItem);

    ListItem->ItemValue = ItemValue;
    ListItem->Container = List;
    List->NumberOfList++;

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

    if (List->Itemindex == (List_Item *)&List->ListEndItem)
    {
        List->Itemindex = List->Itemindex->NextListItem;
    }

    return pdTRUE;
}

/**
 * @brief  列表项删除函数
 * @param  ListItem: 删除的列表项
 * @return rState
 * @note
 */
rState ListItemRemove(List_Item *ListItem)
{
    // /*判断参数合理性*/
    // __is_null__(ListItem);

    ListItem->PreListItem->NextListItem = ListItem->NextListItem;
    ListItem->NextListItem->PreListItem = ListItem->PreListItem;

    ListItem->Container->NumberOfList--;
    if (ListItem->Container->Itemindex == ListItem)
    {
        ListItem->Container->Itemindex = ListItem->NextListItem;
        if ((ListItem->Container->Itemindex == (List_Item *)&((ListItem->Container)->ListEndItem)))
            ListItem->Container->Itemindex = ListItem->Container->Itemindex->NextListItem;
    }

    ListItem->PreListItem = NULL;
    ListItem->NextListItem = NULL;
    ListItem->Container = NULL;

    return pdTRUE;
}

// /**
//  * @brief  列表索引项切换为下一个列表项
//  * @param  List: 列表
//  * @return rState
//  * @note
//  */
// rState ListItemIndexToNext(List_t List)
// {
//     /*判断参数合理性*/
//     __is_null__(List);

//     /*如果下一列表项是哨兵节点则切换为列表的头节点*/
//     if (List->Itemindex->NextListItem != (List_Item *)&List->ListEndItem)
//     {
//         List->Itemindex = List->Itemindex->NextListItem;
//     }
//     else
//     {
//         List->Itemindex = List->ListEndItem.NextListItem;
//     }

//     return pdTRUE;
// }
