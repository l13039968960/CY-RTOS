/*************************************************************************
 * @file     : list.h
 * @brief    : 列表相关函数
 * @author   : zw1194
 * @date     : 2026-02-21
 * @version  : V1.0
 * @copyright:
 * @note     :
 *************************************************************************/

#ifndef __LIST_H__
#define __LIST_H__

#include "project_def.h"

struct List;
typedef struct List_Item
{
    uint64_t ItemValue;
    struct List_Item *NextListItem;
    struct List_Item *PreListItem;
    void *Owner;
    struct List *Container;
} List_Item;

typedef struct MiniList_Item
{
    uint64_t ItemValue;
    struct List_Item *NextListItem;
    struct List_Item *PreListItem;
} MiniList_Item;

typedef struct List
{
    uint64_t NumberOfList;
    List_Item *Itemindex;
    MiniList_Item ListEndItem;
} List;

typedef struct List *List_t;

#define ListGetHeadItem(List) (((List)->ListEndItem).NextListItem)
#define ListGetItemValue(ListItem) ((ListItem)->ItemValue)

/*列表创建函数*/
state_return ListCreat(List_t *List);
/*列表静态创建函数*/
state_return ListCreatStatic(List_t List);
/*列表项插入函数*/
state_return ListItemInsert(List_Item *ListItem, List_t List, uint64_t ItemValue);
/*列表项删除函数*/
state_return ListItemRemove(List_Item *ListItem);

#endif
