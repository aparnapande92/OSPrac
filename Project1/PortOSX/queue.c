/*
 * Generic queue implementation.
 *
 */
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

/*
 * Return an empty queue.
 */
queue_t 
queue_new() {

	struct queue *q = (struct queue *) malloc(sizeof(struct queue)); 

	if (q == NULL) 
	{
		printf("ERROR: new queue memory allocation failed.\n");
		return NULL; 
	}

	q->head = NULL;
	q->tail = NULL;
	q->length = 0; 

    return q;
}

/*
 * Prepend a void* to a queue (both specifed as parameters).  Return
 * 0 (success) or -1 (failure).
 */
int 
queue_prepend(queue_t queue, void* item) {
	struct Node *n; 

	if (queue == NULL || item == NULL) return -1; 


	n = (struct Node *) malloc(sizeof(struct Node)); 
	if (n == NULL) 
	{
		printf("ERROR: queue_prepend memory allocation failed.\n");
		return -1; 
	}

	n->data = item; 
	n->next = queue->head; 

	queue->head = n; 

	//case for empty queue 
	if (queue->length == 0) queue->tail = n; 

	queue->length++; 

    return 0;
}

/*
 * Append a void* to a queue (both specifed as parameters). Return
 * 0 (success) or -1 (failure). 
 */
int queue_append(queue_t queue, void* item) 
{
	struct Node *n;

	if (queue == NULL || item == NULL) return -1; 

	n = (struct Node *) malloc(sizeof(struct Node));	 
	if (n == NULL) 
	{
		printf("ERROR: queue_append memory allocation failed.\n");
		return -1; 
	}

	n->data = item;
	n->next = NULL; 

	if (queue->length > 0)
	{
		queue->tail->next = n; 
		queue->tail = n; 
	}
	else //empty queue  
	{
		queue->head = n;
		queue->tail = n;
	}

	queue->length++; 

    return 0;
}

/*
 * Dequeue and return the first void* from the queue or NULL if queue
 * is empty.  Return 0 (success) or -1 (failure).
 */
int queue_dequeue(queue_t queue, void** item) 
{
  
  struct Node *prevHead; 
	
	if (queue == NULL || item == NULL) return -1; 
  
	if (queue->length > 0)
	{
		*item = queue->head->data;
    prevHead = queue->head; 
		queue->head = queue->head->next;    

		if (queue->length == 1) queue->tail = NULL; 
    
		if (prevHead != NULL) free(prevHead);  
	}
	else 
	{
		*item = NULL;
		return -1; 
	}

  
	queue->length--;   
  return 0;
}

/*
 * Iterate the function parameter over each element in the queue.  The
 * additional void* argument is passed to the function as its first
 * argument and the queue element is the second.  Return 0 (success)
 * or -1 (failure).
 */
int queue_iterate(queue_t queue, PFany f, void* item) {
	struct Node *currentNode;

	if (queue == NULL || f == NULL) return -1; 

	currentNode = queue->head; 

	while (currentNode != NULL)
	{
		if (f(item, currentNode->data) != 0) return -1; 
		
		currentNode = currentNode->next; 
	}

    return 0;
}

/*
 * Free the queue and return 0 (success) or -1 (failure).
 */
int queue_free (queue_t queue) {
	struct Node *currentNode; 
	struct Node *temp;

	if (queue == NULL) return -1; 

	currentNode = queue->head; 

	while (currentNode != NULL)
	{
		temp = currentNode; 
		currentNode = currentNode->next; 

		if (temp != NULL) free(temp); 
	}

	free(queue);
  //printf(",,,, %d",(queue->length));

    return 0;
}

/*
 * Return the number of items in the queue.
 */
int
queue_length(queue_t queue) {
	if (queue == NULL) return -1;

    return queue->length;
}


/* 
 * Delete the specified item from the given queue. 
 * Return -1 on error.
 */
int queue_delete(queue_t queue, void** item) {
	struct Node *curr; 
	struct Node *temp;  	

	if (queue == NULL || item == NULL) return -1;

	curr = queue->head; 

	//Delete Head of queue 
	if (curr->data == *item) 
	{
		queue->head = curr->next; 
		if (queue->length == 1) queue->tail = NULL; 

		free(curr);
		queue->length--; 
		return 1;
	}

	//Search queue for item 
	while (curr->next != NULL)
	{
		if ( *item == curr->next->data)
		{
			temp = curr->next; 
			curr->next = curr->next->next; 

			//Reset tail pointer in case of deleting tail node 
			if (curr->next == NULL) queue->tail = curr; 

			free(temp); 

			queue->length--; 
			return 1; 
		} 

		curr = curr->next; 
	}

	// NOT SURE ON THIS if it should return error if it cannot find Item in queue 
    return -1;
}
