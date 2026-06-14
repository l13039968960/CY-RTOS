#include "../include/messagequeue.h"
#include "../include/queue_prv.h"

/*消息队列*/
struct MessageQueue
{
	QueueBase_t Base; /*队列基类*/
};

/*消息队列虚函数表*/
static const Queue_ops_t MessgaeQueue_ops = {
	.vQueueDeInit = vMessageQueueDestory,
};

pMessageQueue_t xMessageQueueCreate(BaseType_t MessageNum, BaseType_t MessageSize)
{
	BaseState_t state;
	pMessageQueue_t this;

	/*分配句柄空间*/
	state = sHeapMemAllocate(sizeof(pMessageQueue_t), (pStackType_t *)&this);
	if (state != pdTRUE)
		this = NULL;
	else
	{
		/*分配缓冲区空间*/
		state = sHeapMemAllocate(MessageNum * MessageSize, (pStackType_t *)&this->Storage);
		if (state != pdTRUE)
		{
			/*失败则释放句柄空间*/
			sHeapMemFree((pStackType_t)this);
			this = NULL;
		}
		else
		{
			/*QueueBase初始化*/
			this->Base.Queue_Count = 0;

			this->Base.vptr = &MessgaeQueue_ops;

			sListCreatStatic(&this->Base.Queue_RxEventList);
			sListCreatStatic(&this->Base.Queue_TxEventList);

			/*MessageQueue初始化*/
			this->MessageNum = MessageNum;
			this->MessageSize = MessageSize;

			this->StorageTail = (pQueueType_t *)((uint32_t)this->StorageTail + MessageNum * MessageSize);

			this->pRead = this->StorageTail;
			this->pWrite = this->StorageTail;
		}
	}
}

void vMessageQueueDestory(pMessageQueue_t this)
{
	sHeapMemFree((pStackType_t)this->Storage);
	sHeapMemFree((pStackType_t)this);
}
