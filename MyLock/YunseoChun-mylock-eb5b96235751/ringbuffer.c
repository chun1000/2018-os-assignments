#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "queue.h"
#include "config.h"
#include "locks.h"

static int nr_slots = 0;

static enum lock_types lock_type;
struct spinlock slock;
struct mutex m;
struct semaphore sem;

void (*enqueue_fn)(int value) = NULL;
int (*dequeue_fn)(void) = NULL;

int *ringbuffer; int front = 0; int rear = 0; int max = 1;

int isEmpty()
{
	if(front == rear) return 1;
	else return 0;
}
int isFull()
{
	if((rear+1)%max == front) return 1;
	else return 0;
}

void enqueue_ringbuffer(int value)
{
	assert(enqueue_fn);
	assert(value >= MIN_VALUE && value < MAX_VALUE);

	enqueue_fn(value);
}

int dequeue_ringbuffer(void)
{
	int value;

	assert(dequeue_fn);

	value = dequeue_fn();
	assert(value >= MIN_VALUE && value < MAX_VALUE);

	return value;
}


/*********************************************************************
 * TODO: Implement using spinlock
 */
void enqueue_using_spinlock(int value)
{
	while(1)
	{
		acquire_spinlock(&slock);
		if(isFull()) release_spinlock(&slock);
		else
		{
			rear = (rear+1) %max;
			ringbuffer[rear] = value;
			release_spinlock(&slock);
			return;
		}
	}
}

int dequeue_using_spinlock(void)
{
	int buffer; 
	while(1)
	{
		acquire_spinlock(&slock);
		if(isEmpty()) release_spinlock(&slock);
		else
		{
			front = (front+1) % max;
			buffer = ringbuffer[front];
			release_spinlock(&slock);
			return buffer;
		}
	}
	
    
}

void init_using_spinlock(void)
{
	enqueue_fn = &enqueue_using_spinlock;
	dequeue_fn = &dequeue_using_spinlock;
	
	init_spinlock(&slock);
}

void fini_using_spinlock(void)
{

}


/*********************************************************************
 * TODO: Implement using mutex
 */
void enqueue_using_mutex(int value)
{
	while(1)
	{
		acquire_mutex(&m);
		if(isFull()) release_mutex(&m);
		else
		{
			rear = (rear+1) %max;
			ringbuffer[rear] = value;
			release_mutex(&m);
			return;
		}
	}
}

int dequeue_using_mutex(void)
{
	int buffer;
	while(1)
	{
		acquire_mutex(&m);
		if(isEmpty()) release_mutex(&m);
		else
		{
			front = (front+1) % max;
			buffer = ringbuffer[front];
			release_mutex(&m);
			return buffer;
		}
	}
}

void init_using_mutex(void)
{
	enqueue_fn = &enqueue_using_mutex;
	dequeue_fn = &dequeue_using_mutex;
	init_mutex(&m);
}

void fini_using_mutex(void)
{
}


/*********************************************************************
 * TODO: Implement using semaphore
 */
void enqueue_using_semaphore(int value)
{
	while(1)
	{
		wait_semaphore(&sem);
		if(isFull()) signal_semaphore(&sem);
		else
		{
			rear = (rear+1) %max;
			ringbuffer[rear] = value;
			signal_semaphore(&sem);
			return;
		}
	}
}

int dequeue_using_semaphore(void)
{
	int buffer;
	while(1)
	{
		wait_semaphore(&sem);
		if(isEmpty()) signal_semaphore(&sem);
		else
		{
			front = (front+1) % max;
			buffer = ringbuffer[front];
			signal_semaphore(&sem);
			return buffer;
		}
	}
}

void init_using_semaphore(void)
{
	enqueue_fn = &enqueue_using_semaphore;
	dequeue_fn = &dequeue_using_semaphore;
	init_semaphore(&sem, 1);
}

void fini_using_semaphore(void)
{
}


/*********************************************************************
 * Common implementation
 */
int init_ringbuffer(const int _nr_slots_, const enum lock_types _lock_type_)
{
	assert(_nr_slots_ > 0);
	nr_slots = _nr_slots_;

	/* Initialize lock! */
	lock_type = _lock_type_;
	switch (lock_type) {
	case lock_spinlock:
		init_using_spinlock();
		break;
	case lock_mutex:
		init_using_mutex();
		break;
	case lock_semaphore:
		init_using_semaphore();
		break;
	}

	ringbuffer = (int*)malloc(sizeof(int)*_nr_slots_);
	max = _nr_slots_+1;

	return 0;
}

void fini_ringbuffer(void)
{
	/* TODO: Clean up what you allocated */
	free(ringbuffer);
	switch (lock_type) {
	default:
		break;
	}
}
