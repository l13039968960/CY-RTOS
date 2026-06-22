#ifndef __OS_PRV_H__
#define __OS_PRV_H__

#include "../../../include/os.h"

extern pTCB_t OSCurrentTCB;		 /*当前任务TCB*/
extern List_t ReadyTaskList[32]; /*就绪任务列表*/
extern List_t DelayTaskList;	 /*延时阻塞任务列表*/
extern BaseType_t OSCurrentTick; /*当前定时器计数器*/
extern pTCB_t OSNextTCB;

#endif
