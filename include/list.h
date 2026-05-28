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

#include "os_config.h"
#include "project_def.h"

#define sListGetHeadItem(List) (((List)->ListEndItem).NextListItem)
#define sListGetItemValue(ListItem) ((ListItem)->ItemValue)

struct List;
struct ListItem
{
	uint64_t ItemValue;

	struct ListItem *NextListItem;
	struct ListItem *PreListItem;

	void *Owner;
	struct List *Container;
};

struct MiniListItem
{
	uint64_t ItemValue;

	struct ListItem *NextListItem;
	struct ListItem *PreListItem;
};

struct List
{
	uint64_t NumberOfList;

	struct ListItem *Itemindex;

	struct MiniListItem ListEndItem;
};

typedef struct ListItem ListItem_t;
typedef struct MiniListItem MiniListItem_t;
typedef struct List List_t;

typedef ListItem_t *pListItem_t;
typedef List_t *pList_t;

#include "heap.h"

/**
 * @brief  列表创建函数
 * @param  List: 创建的列表
 * @return xTRUE:创建成功
 *         xFALSE:创建失败
 * @note
 */
BaseState_t sListCreat(pList_t *List);

/**
 * @brief  静态列表创建函数
 * @param  List: 创建的列表
 * @return xTRUE:创建成功
 *         xFALSE:创建失败
 * @note
 */
BaseState_t sListCreatStatic(pList_t List);

/**
 * @brief  列表项插入函数
 * @param  ListItem: 插入的列表项
 * @param  List: 被插入的列表
 * @param  ItemValue: 列表项的值
 * @return xTRUE:插入成功
 *         xFALSE:插入失败
 * @note   列表项升序排列
 */
BaseState_t sListItemInsert(pListItem_t ListItem, pList_t List, BaseType_t ItemValue);

/**
 * @brief  列表项删除函数
 * @param  ListItem: 删除的列表项
 * @return xTRUE:插入成功
 *         xFALSE:插入失败
 * @note
 */
BaseState_t sListItemRemove(pListItem_t ListItem, pList_t List);

/**
 * @brief  列表索引项切换为下一个列表项
 * @param  List: 列表
 * @return BaseState_t
 * @note
 */
BaseState_t sListIndexToNext(pList_t List);

/**
 * @brief  获取列表索引列表项
 * @param  List: 列表
 * @param  ListItem:列表项指针
 * @return BaseState_t
 * @note   ListItem为NULL代表列表为空
 */
BaseState_t sListGetIndexItem(pList_t List, pListItem_t* ListItem);

#endif
