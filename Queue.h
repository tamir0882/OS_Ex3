#ifndef __QUEUE_H__
#define __QUEUE_H__
#include "HardCodedData.h"
#include <stdbool.h> 

Queue* InitializeQueue();

bool Empty(Queue* q);

void Push(Queue* q, Element* p_element);

Element* Top(Queue* q);

int Pop(Queue* q);

void DestroyQueue(Queue* q);


#endif //__QUEUE_h__