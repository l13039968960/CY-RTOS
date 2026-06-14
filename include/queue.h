#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef struct Queue Queue_t;
typedef Queue_t *pQueue_t;

pQueue_t xQueueCreate(BaseType_t MessageNum, BaseType_t MessageSize);

#endif
