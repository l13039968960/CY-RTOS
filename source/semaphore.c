#include "../include/semaphore.h"

BaseState_t sSemaphoreCreateBinary(pSemaphore_t *SemaphoreHandler)
{
	BaseState_t xstate;
	xstate = sHeapMemAllocate(sizeof(Semaphore_t), (pStackType_t *)SemaphoreHandler);
	if (xstate == pdTRUE)
	{
		(*SemaphoreHandler)->Semaphore_Counting = 0;
		(*SemaphoreHandler)->Semaphore_ItemValue = 0;

		sListCreatStatic(&((*SemaphoreHandler)->Semaphore_RxEventList));
		sListCreatStatic(&((*SemaphoreHandler)->Semaphore_RxEventList));
	}
	else
	{
		return pdFALSE;
	}
	return pdTRUE;
}

BaseState_t sSemaphoreDeleteBinary(pSemaphore_t *SemaphoreHandler)
{
	BaseState_t xstate;
	xstate = sHeapMemFree((StackType_t *)SemaphoreHandler);
	if (xstate != pdTRUE)
		return pdFALSE;

	return pdTRUE;
}

BaseState_t sSemaphoreCreateCounting(pSemaphore_t *SemaphoreHandler)
{
	BaseState_t xstate;
	xstate = sHeapMemAllocate(sizeof(Semaphore_t), (pStackType_t *)SemaphoreHandler);
	if (xstate == pdTRUE)
	{
		(*SemaphoreHandler)->Semaphore_Counting = 0;
		(*SemaphoreHandler)->Semaphore_ItemValue = 0;

		sListCreatStatic(&((*SemaphoreHandler)->Semaphore_RxEventList));
		sListCreatStatic(&((*SemaphoreHandler)->Semaphore_RxEventList));
	}
	else
	{
		return pdFALSE;
	}
	return pdTRUE;
}

BaseState_t sSemaphoreDeleteCounting(pSemaphore_t *SemaphoreHandler)
{
	BaseState_t xstate;
	xstate = sHeapMemFree((StackType_t *)SemaphoreHandler);
	if (xstate != pdTRUE)
		return pdFALSE;

	return pdTRUE;
}

BaseState_t sSemaphoreCreateMutex(pSemaphore_t *SemaphoreHandler)
{
	BaseState_t xstate;
	xstate = sHeapMemAllocate(sizeof(Semaphore_t), (pStackType_t *)SemaphoreHandler);
	if (xstate == pdTRUE)
	{
		(*SemaphoreHandler)->Semaphore_Counting = 0;
		(*SemaphoreHandler)->Semaphore_ItemValue = 0;

		sListCreatStatic(&((*SemaphoreHandler)->Semaphore_RxEventList));
		sListCreatStatic(&((*SemaphoreHandler)->Semaphore_RxEventList));
	}
	else
	{
		return pdFALSE;
	}
	return pdTRUE;
}

BaseState_t sSemaphoreDeleteMutex(pSemaphore_t *SemaphoreHandler)
{
	BaseState_t xstate;
	xstate = sHeapMemFree((StackType_t *)SemaphoreHandler);
	if (xstate != pdTRUE)
		return pdFALSE;

	return pdTRUE;
}