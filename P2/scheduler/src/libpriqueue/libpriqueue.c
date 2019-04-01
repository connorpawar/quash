/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "libpriqueue.h"

void* deleteNode(node_t* del){
	void* obj = del->m_obj;
	free(del);
	return(obj);
}

/**
  Initializes the priqueue_t data structure.

  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
	q->m_size = 0;
	q->m_front = NULL;
	q->m_comparer = comparer;
}


/**
  Inserts the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{

	node_t * add = (node_t*)malloc(sizeof(node_t));
	add->m_obj = ptr;
	add->m_next = NULL;
	node_t * temp = q->m_front;
	if(q->m_size == 0){
		q->m_size =1;
		q->m_front = add;
		return 0;
	}

	if(q->m_comparer(add->m_obj,q->m_front->m_obj)<0){
		add->m_next = q->m_front;
		q->m_front = add;
		q->m_size++;
		return 0;
	}
	for(int i = 0; i < q->m_size; i++){
		if(temp->m_next == NULL || q->m_comparer(add->m_obj,temp->m_next->m_obj)<0){
			add->m_next = temp->m_next;
			temp->m_next = add;
			q->m_size ++;
			return(i++);
		}
		temp=temp->m_next;
	}

    exit(1);
	return -1;
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
	if(q->m_size!=0){
		return(q->m_front->m_obj);
	}
	return NULL;
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
	if(q->m_size == 0){
		return(NULL);
	}
	else{
		node_t * temp = q->m_front;
		q->m_front = temp->m_next;
		q->m_size--;
		return(deleteNode(temp));
	}
}


/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void *priqueue_at(priqueue_t *q, int index)
{
	if(index<0){
		return(NULL);
	}
	else if(index>=q->m_size){
		return(NULL);
	}
	node_t * temp = q->m_front;
	for(int i = 1; i<=index; i++){
		temp = temp->m_next;
	}
	return(temp->m_obj);
}


/**
  Removes all instances of ptr from the queue.

  This function should not use the comparer function, but check if the data contained in each element of the queue is equal (==) to ptr.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
	int totalrem = 0;

	while(q->m_size>0 && ptr == q->m_front->m_obj){
		priqueue_poll(q);
		totalrem++;
	}

	node_t * temp = q->m_front;
	node_t * temp2;
	if(q->m_size!=0){
		while(temp->m_next != NULL){
			if(temp->m_next->m_obj==ptr){
				temp2=temp->m_next;
				temp->m_next = temp2->m_next;
				deleteNode(temp2);
				q->m_size--;
				totalrem++;
			}
			else{
				temp = temp->m_next;
			}
		}
	}
	return(totalrem);
}


/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
	if(index<0){
		return NULL;
	}
	if(index>=q->m_size){
		return NULL;
	}
	else if(index == 0){
		return(priqueue_poll(q));
	}
	else{
		node_t * temp = q->m_front;
		for(int i = 1; i<index; i++){
			temp = temp->m_next;
		}
		node_t * temp2 = temp->m_next;
		temp->m_next = temp2->m_next;
		q->m_size--;
		return(deleteNode(temp2));
	}
}


/**
  Returns the number of elements in the queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
	return q->m_size;
}


/**
  Destroys and frees all the memory associated with q.

  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{
	while(priqueue_poll(q) != NULL){

	}
}
