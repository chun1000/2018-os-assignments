#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "config.h"
#include "locks.h"
#include "atomic.h"

/******************************************************************
 * Spinlock implementation
 */
void init_spinlock(struct spinlock *lock)
{
	lock->hold = 0;
	return;
}

void acquire_spinlock(struct spinlock *lock)
{
	while(compare_and_swap(&lock->hold, 0, 1));
	return;
}

void release_spinlock(struct spinlock *lock)
{
	compare_and_swap(&lock->hold, 1, 0);
	return;
}


/******************************************************************
 * Blocking lock implementation
 *
 * Hint: Use pthread_self, pthread_kill, pause, and signal
 */

void sHandler(int i)
{
	//just use for signal
}

void init_mutex(struct mutex *lock)
{
	lock->hold = 0; //mutex hold
	lock->m_slock.hold = 0; //mutex's spinlock.
	TAILQ_INIT(&thread.head); 
	signal(SIGUSR1, sHandler); //SIGUSR1 for awake thread
	return;
}

void acquire_mutex(struct mutex *lock)
{
	acquire_spinlock(&(lock->m_slock));
	if(lock->hold == 1) //if one thread executed
	{
		struct thread *p;
		p = (struct thread*)malloc(sizeof(struct thread)); 
		p->pthread = pthread_self();
		p->flags = 0;

		TAILQ_INSERT_TAIL(&thread.head, p, entries); 
		release_spinlock(&(lock->m_slock));
		pause(); //Insert thread in Queue and Sleep.
		p->flags = 1; //wake flag.
	}
	else
	{
		lock->hold = 1;
		release_spinlock(&(lock->m_slock));
	}
}

void release_mutex(struct mutex *lock)
{
	acquire_spinlock(&lock->m_slock);

	if(!TAILQ_EMPTY(&thread.head))
	{
		struct thread *p;
		p = TAILQ_FIRST(&thread.head);
		TAILQ_REMOVE(&thread.head, p, entries);
		release_spinlock(&lock->m_slock); //protect queue deletion.

		while(1)
		{
			pthread_kill(p->pthread, SIGUSR1); //kill while thread wake.
			usleep(500);
			if(p->flags == 1) //when thread awake.
			{
				free(p); //free awake thread memory.
				break;
			} 
		}
		
	}
	else
	{
		lock->hold = 0;
		release_spinlock(&lock->m_slock);
	}
	return;
}


/******************************************************************
 * Semaphore implementation
 *
 * Hint: Use pthread_self, pthread_kill, pause, and signal
 */
void init_semaphore(struct semaphore *sem, int S)
{
	sem->value = S; 
	TAILQ_INIT(&thread.head);
	signal(SIGUSR1, sHandler);
	return;
}

void wait_semaphore(struct semaphore *sem)
{
	acquire_spinlock(&(sem->m_slock));
	sem->value--;

	if(sem->value < 0)
	{
		struct thread *p;
		p = (struct thread*)malloc(sizeof(struct thread));
		p->pthread = pthread_self();
		p->flags = 0;
		TAILQ_INSERT_TAIL(&thread.head, p, entries);
		release_spinlock(&(sem->m_slock));
		pause(); // insert in queue and sleep.

		p->flags = 1; //wake up flag.
	}
	else release_spinlock(&(sem->m_slock));
	return;
}

void signal_semaphore(struct semaphore *sem)
{
	acquire_spinlock(&sem->m_slock);

	sem->value++;
	if(!TAILQ_EMPTY(&thread.head))
	{
		struct thread *p;
		p = TAILQ_FIRST(&thread.head);
		TAILQ_REMOVE(&thread.head, p, entries);

		release_spinlock(&sem->m_slock); //protect queue

		while(1)
		{
			pthread_kill(p->pthread, SIGUSR1); //kill while thread sleep.
			usleep(500);
			if(p->flags == 1) //thread wake up.
			{
				free(p);
				break;
			} 
		}
		
	}
	else release_spinlock(&sem->m_slock);

	return;
}


/******************************************************************
 * Spinlock tester exmaple
 */
struct spinlock testlock;
int testlock_held = 0;

void *test_thread(void *_arg_)
{
	usleep(random() % 1000 * 1000);

	printf("Tester acquiring the lock...\n");
	acquire_spinlock(&testlock);
	printf("Tester acquired\n");
	assert(testlock_held == 0);
	testlock_held = 1;

	sleep(1);

	printf("Tester releases the lock\n");
	testlock_held = 0;
	release_spinlock(&testlock);
	printf("Tester released the lock\n");
	return 0;
}

void test_lock(void)
{
	/* Set nr_testers as you need
	 *  1: one main, one tester. easy :-)
	 * 16: one main, 16 testers contending the lock :-$
	 */
	const int nr_testers = 1;
	int i;
	pthread_t tester[nr_testers];

	printf("Main initializes the lock\n");
	init_spinlock(&testlock);

	printf("Main graps the lock...");
	acquire_spinlock(&testlock);
	assert(testlock_held == 0);
	testlock_held = 1;
	printf("acquired!\n");

	for (i = 0; i < nr_testers; i++) {
		pthread_create(tester + i, NULL, test_thread, NULL);
	}

	sleep(1);

	printf("Main releases the lock\n");
	testlock_held = 0;
	release_spinlock(&testlock);
	printf("Main released the lock\n");

	for (i = 0; i < nr_testers; i++) {
		pthread_join(tester[i], NULL);
	}
	assert(testlock_held == 0);
	printf("Your spinlock implementation looks O.K.\n");

	return;
}

