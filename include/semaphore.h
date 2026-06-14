#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include "os_config.h"
#include "project_def.h"
#include "list.h"
#include "heap.h"

/*信号量结构体定义*/
struct Semaphore
{
	BaseType_t Semaphore_Counting; /*信号量计数*/

	BaseType_t Semaphore_ItemValue; /*其他保存数据*/

	List_t Semaphore_RxEventList; /*队列接收阻塞列表*/
	List_t Semaphore_TxEventList; /*队列发送阻塞列表*/
};

typedef uint8_t SemaphoreType_t;
typedef SemaphoreType_t *pSemaphoreType_t;

typedef struct Semaphore Semaphore_t;
typedef Semaphore_t *pSemaphore_t;



#endif
