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
		printf("InitializeQueue: Memory Error - Couldn't initialize queue.\n");
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
	return q->front;
}

void Pop(Queue* q)
{
	if (Empty(q))
	{
		printf("Queue is empty, nothing to pop");
		return;
	}

	Element* temp = NULL;
	temp = q->front;
	q->front = q->front->next;
	free(temp);

	if (NULL == q->front)
	{
		q->rear = NULL;
	}
}

void DestroyQueue(Queue* q)
{
	while (!Empty(q))
	{
		Pop(q);
	}
	free(q);
	q = NULL;
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