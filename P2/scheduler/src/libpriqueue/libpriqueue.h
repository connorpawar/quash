/** @file libpriqueue.h
 */

#ifndef LIBPRIQUEUE_H_
#define LIBPRIQUEUE_H_

typedef struct node{
	void * m_obj;
	struct node * m_next;
}node_t;//node for the queue to hold on to

void* deleteNode(node_t* del);

/**
  Priqueue Data Structure
*/
typedef struct _priqueue_t
{
	int m_size;
	struct node * m_front;
	int (*m_comparer)(const void *, const void *);
} priqueue_t;


void   priqueue_init     (priqueue_t *q, int(*comparer)(const void *, const void *));

int    priqueue_offer    (priqueue_t *q, void *ptr);
void * priqueue_peek     (priqueue_t *q);
void * priqueue_poll     (priqueue_t *q);
void * priqueue_at       (priqueue_t *q, int index);
int    priqueue_remove   (priqueue_t *q, void *ptr);
void * priqueue_remove_at(priqueue_t *q, int index);
int    priqueue_size     (priqueue_t *q);

void   priqueue_destroy  (priqueue_t *q);

#endif /* LIBPQUEUE_H_ */
