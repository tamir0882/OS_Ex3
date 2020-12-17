#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "HardCodedData.h"

// For the implementation of the queue we used a code from the website
//https://www.geeksforgeeks.org/queue-linked-list-implementation/



Queue* InitializeQueue()
{
	Queue* new_q = (Queue*)malloc(sizeof(Queue));
	if (new_q == NULL)
	{
		printf("Memory Error - Couldn't initialize queue. ABORT.");
		return NULL;
	}
	new_q->front = NULL;
	new_q->rear = NULL;
	return new_q;
}

bool Empty(Queue* q)
{
	if (q->front == NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

Element* Top(Queue* q)
{
	if (Empty(q))
	{
		printf("Queue is empty.");
		return NULL;
	}
	printf("returns element with index: %d\n", q->front->index);
	return q->front;
}

int Pop(Queue* q)
{
	if (Empty(q))
	{
		printf("Can not Pop. Queue is empty.");
		return FAILURE;
	}

	q->front = q->front->next;

	if (NULL == q->front)
	{
		q->rear = NULL;
	}
	return SUCCESS;
}

int DestoryQueue(Queue* q)
{
	while (Empty(q) != true)
	{
		Pop(q);
		printf("Popped!\n");
	}
	free(q);
	q = NULL;
	printf("Queue has been destroyed successfully!\n");
	return SUCCESS;
}

void Push(Queue* q, Element* p_element)
{
	if (Empty(q))
	{
		q->front = p_element;
		q->front->next = NULL;
		q->rear = q->front;
		q->rear->next = NULL;
		return;
	}
	else
	{
		Element* temp = q->front;
		while (temp != NULL)
		{
			if (p_element == temp)
			{
				printf("This element is already in the queue, returning.\n");
				return;
			}
			temp = temp->next;
		}
		q->rear->next = p_element;
		q->rear = p_element;
		q->rear->next = NULL;
		return;
	}
}