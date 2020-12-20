#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "HardCodedData.h"

// For the implementation of the queue we used a code from the website
//https://www.geeksforgeeks.org/queue-linked-list-implementation/

//Description - this function initialize space in memory for a new queue.
//Parameters - No input parameters. The output parameter is a pointer to
//the new initialized queue.
//Returns - pointer to a struct of type Queue.

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

//Description - this function checks if the given queue has elements and 
//returns False or True if is empty.
//Parameters - The input parameter is a pointer to a queue of type struct Queue. 
//The output paramater is a boolian type indacation of empty queue.
//Returns - Boolian value.

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

//Description - this function finds the first element in queue id not empty and returns it.
//Parameters - The input parameter is a pointer to a queue of type struct Queue. 
//The output paramater is a pointer to an element of type Element.
//Returns - an element of type Element.

Element* Top(Queue* q)
{
	if (Empty(q))
	{
		printf("Queue is empty.\n");
		return NULL;
	}
	return q->front;
}

//Description - this function finds the first element in queue, takes it out and 
//sets the one after it to be first.
//Parameters - The input parameter is a pointer to a queue of type struct Queue. 
//The output paramater is a pointer to an element of type Element.
//Returns - an element of type Element.

void Pop(Queue* q)
{
	if (Empty(q))
	{
		printf("Queue is empty, nothing to pop.\n");
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
