/*
 * minithread.c:
 *	This file provides a few function headers for the procedures that
 *	you are required to implement for the minithread assignment.
 *
 *	EXCEPT WHERE NOTED YOUR IMPLEMENTATION MUST CONFORM TO THE
 *	NAMING AND TYPING OF THESE PROCEDURES.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include "minithread.h"
#include "queue.h"
#include "synch.h"

#include <assert.h>

 int count = 0; 
 queue_t ready_queue;
 queue_t cleaning_queue;  

 int scheduled_cleanup = 0; 

 minithread_t current_thread;
 minithread_t reaper_thread; 
 minithread_t idle_thread;  


/*
 * A minithread should be defined either in this file or in a private
 * header file.  Minithreads have a stack pointer with to make procedure
 * calls, a stackbase which points to the bottom of the procedure
 * call stack, the ability to be enqueueed and dequeued, and any other state
 * that you feel they must have.
 */


/* minithread functions */

minithread_t
minithread_fork(proc_t proc, arg_t arg) 
{
	minithread_t new_mt;

	new_mt = minithread_create(proc, arg); 
    
	if (queue_append(ready_queue, new_mt) != 0) printf("Error in fork");

    return new_mt;
}

int
reaper_proc(arg_t arg)
{ 
	minithread_t mt;
	while (1)
	{
		while(cleaning_queue->head != NULL)
		{
			queue_dequeue(cleaning_queue, &mt);

			minithread_free_stack(mt->stack_base); 
			free(mt); 
		}
		minithread_yield(); 
	}

	return 0;	
}

int
final_proc(arg_t arg)
{
	minithread_t mt;
	mt = minithread_self();

	if (queue_append(cleaning_queue, mt) != 0) printf("Error in final_proc");

	if (!scheduled_cleanup)
	{
		if (queue_append(ready_queue, reaper_thread) != 0) printf("Error in final_proc scheduling idle queue");	
	}

	minithread_stop();

	return 0; 
}

int
idle_proc(arg_t arg)
{
	while (1) 
	{
		printf("In Final proc");
		minithread_stop();
	}

	return 0; 
}

minithread_t
minithread_create(proc_t proc, arg_t arg) {
    minithread_t mt;
	stack_pointer_t top;
	stack_pointer_t base;  
	minithread_allocate_stack(&base, &top);
	
	mt = (minithread_t) malloc(sizeof(struct minithread));
	
	mt->stack_top = top;
	mt->stack_base = base; 
	mt->id = count++; 

	minithread_initialize_stack(&top, proc, arg, &final_proc, NULL);

    return mt;
}

minithread_t
minithread_self() {

    return current_thread;
}

int
minithread_id() {
	minithread_t mt;
	mt = minithread_self();

    return mt->id;
}

void
minithread_stop() {
	minithread_t mt; 
	minithread_t thread_to_run; 
	
	mt = minithread_self(); 

	queue_dequeue(ready_queue, &thread_to_run);
	printf("%p\n", thread_to_run);

	//If thread is empty switch to idle_thread
	if (thread_to_run == NULL)
	{
		current_thread = idle_thread; 
		minithread_switch(&(mt->stack_top), &(idle_thread->stack_top)); 
	}
	else
	{
		current_thread = thread_to_run;
		printf("&&&%p\n", current_thread);
		minithread_switch(&(mt->stack_top), &(thread_to_run->stack_top));	
		printf("****\n");
	}
}

void
minithread_start(minithread_t t) {
	if (queue_append(ready_queue, t) != 0) printf("Error in start");
}

void
minithread_yield() {
	minithread_t mt; 
	minithread_t thread_to_run; 
	
	mt = minithread_self(); 
	printf(" %d \n", mt->id );
   
	queue_dequeue(ready_queue, &thread_to_run);
	printf(" %d \n", thread_to_run->id );
	printf(" %p \n", thread_to_run->stack_top );

	//If thread is empty switch to idle_thread
	if (thread_to_run == NULL)
	{
		current_thread = idle_thread; 
		if (queue_append(ready_queue, mt) != 0) printf("Error in yield");
		minithread_switch(&(mt->stack_top), &(idle_thread->stack_top)); 
	}
	else
	{
		current_thread = thread_to_run;
        
		if (queue_append(ready_queue, mt) != 0) 

		printf(" %p \n", mt->stack_top );	
		printf(" %p \n", thread_to_run->stack_top );	
		minithread_switch(&(mt->stack_top), &(thread_to_run->stack_top));	
	}
}

/*
 * Initialization.
 *
 * 	minithread_system_initialize:
 *	 This procedure should be called from your C main procedure
 *	 to turn a single threaded UNIX process into a multithreaded
 *	 program.
 *
 *	 Initialize any private data structures.
 * 	 Create the idle thread.
 *       Fork the thread which should call mainproc(mainarg)
 * 	 Start scheduling.
 *
 */
void
minithread_system_initialize(proc_t mainproc, arg_t mainarg) 
{
	minithread_t main_thread;

	//Initialize queues  
	cleaning_queue = queue_new(); 
	ready_queue = queue_new(); 
    
	reaper_thread = minithread_create(&reaper_proc, NULL);

	idle_thread = minithread_create(&idle_proc, NULL); 
	current_thread = idle_thread; 

	main_thread = minithread_fork(mainproc, mainarg);

	printf ("about to switch...\n");
	//minithread_switch(&(idle_thread->stack_top), &())


	minithread_yield();

	




 

}


