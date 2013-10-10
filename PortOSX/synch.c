#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "synch.h"
#include "queue.h"
#include "minithread.h"

/*
 *	You must implement the procedures and types defined in this interface.
 */


/*
 * Semaphores.
 */
struct semaphore 
{
    int lock;
    int count;
    queue_t waitQ;
};


/*
 * semaphore_t semaphore_create()
 *	Allocate a new semaphore.
 */
semaphore_t semaphore_create() 
{
    queue_t q;
    q = queue_new();
    struct semaphore *s;
    s = (struct semaphore *) malloc(sizeof(struct semaphore));
    s->lock = 0;
    s->count = 1;
    s->waitQ = q;
    return s;
}

/*
 * semaphore_destroy(semaphore_t sem);
 *	Deallocate a semaphore.
 */
void semaphore_destroy(semaphore_t sem) 
{
    while(queue_free(sem->waitQ) != 0) {}
    sem->lock = 0;
    sem->count = 0;
    free(sem);
}

 
/*
 * semaphore_initialize(semaphore_t sem, int cnt)
 *	initialize the semaphore data structure pointed at by
 *	sem with an initial value cnt.
 */
void semaphore_initialize(semaphore_t sem, int cnt) 
{
    sem->count = cnt;
}


/*
 * semaphore_P(semaphore_t sem)
 *	P on the sempahore.
 */
void semaphore_P(semaphore_t sem) 
{
    while((sem->lock) != 0) {} // ************** This might not be necessary
    sem->lock = 1;
    sem->count = sem->count - 1;
    if (sem->count < 0)
    {
        // add thread to waitQ, why is it that &minithread_self() works?
        // Check pointer stuff!
        minithread_t thr = minithread_self();
        queue_append((sem->waitQ), thr);
        
        sem->lock = 0;
        minithread_stop();
    }
    else
    {
        sem->lock = 0;
    }
    
}

/*
 * semaphore_V(semaphore_t sem)
 *	V on the sempahore.
 */
void semaphore_V(semaphore_t sem) 
{
    int check;
    minithread_t thr;
    while((sem->lock) != 0) {}
    sem->lock = 1;
    sem->count = sem->count + 1;
    if (sem->count <= 0)
    {
        // Remove a thread from the wait queue and start it 
        check = queue_dequeue((sem->waitQ), &thr);
        if (check == 0)
        {
            minithread_start(thr);
        }
    }
    sem->lock = 0;    
}
