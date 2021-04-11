#ifndef __LOCKS_H__
#define __LOCKS_H__
enum lock_types;
#include "queue.h"

/**
 * Have a look at https://linux.die.net/man/3/list_head for using list_head
 */
#include <pthread.h>
struct thread {
	pthread_t pthread;
	unsigned long flags;
	TAILQ_ENTRY(thread) entries;
	TAILQ_HEAD(, thread) head;
} thread;

/*************************************************
 * Spinlock
 */
struct spinlock {
	int hold;
};
void init_spinlock(struct spinlock *);
void acquire_spinlock(struct spinlock *);
void release_spinlock(struct spinlock *);


/*************************************************
 * Mutex
 */
struct mutex {
	int hold;
	struct spinlock m_slock;
};
void init_mutex(struct mutex *);
void acquire_mutex(struct mutex *);
void release_mutex(struct mutex *);


/*************************************************
 * Semaphore
 */
struct semaphore {
	int value;
	struct spinlock m_slock;
};
void init_semaphore(struct semaphore *, const int);
void wait_semaphore(struct semaphore *);
void signal_semaphore(struct semaphore *);

/*************************************************
 * Lock tester.
 * Will be invoked if the program is run with -T
 */
void test_lock(void);
#endif
