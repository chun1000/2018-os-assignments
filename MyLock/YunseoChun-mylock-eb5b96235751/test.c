#include "queue.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "locks.h"
#include "ringbuffer.h"
#include <signal.h>

enum lock_types {
	lock_spinlock = 0,
	lock_mutex = 1,
	lock_semaphore = 2,
};
struct spinlock slock;

pthread_t p[2];

void threadTest(void *data)
{
    printf("thread[%d] : sleep \n", pthread_self());
    pause();
    printf("thread[%d] : awake \n", pthread_self());
}

void threadawake(void *data)
{
    printf("thread[%d] : kill \n", pthread_self());
    pthread_kill(p[0], SIGUSR1);
}
void signal_handler(int i)
{

}

int main()
{
    signal(SIGUSR1, signal_handler);
    pthread_create(&p[0], NULL, threadTest, NULL);
    sleep(1);
    pthread_create(&p[1], NULL, threadawake, NULL);

    pthread_join(p[0], NULL);

    /*
    pthread_t p[10];
    enum lock_types lo = lock_spinlock;
    init_using_mutex();
    init_ringbuffer(11, lo);
    
    printf("init\n");
    for(int i =0; i<10; i++) enqueue_ringbuffer(i);
    for(int i =0; i< 10; i++)printf("%d", dequeue_ringbuffer());
    */
}