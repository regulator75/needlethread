#include "pthread.h"
#define _BITS_PTHREADTYPES_COMMON_H
#include <stdlib.h>
#include <errno.h>
#ifndef __PTHREAD_NEEDLETHREAD_H__
#error "Wrong pthread included"
#endif
#include <stdio.h>

/**
 * Local types
 */

typedef struct _tagthreadData {
	int64_t * stack;
	int64_t * top_of_stack;

	int detached_state;

	// For a thread being waited on by join
	struct _tagthreadData * join_waited_on_by; // If any thread is joining on me, this points there.
	int                     join_this_thread_is_done; // Set to 1 in pthread_exit
	void *                  join_this_thread_retval;  // Store return value passed in pthread_exit
} threadData;

typedef int lowlevel_mutex_lock;

/** 
 * Local data
 */


static threadData* all_threads[NEEDLETHREAD_MAX_THREADS] = {0};
static lowlevel_mutex_lock all_threads_lock = 0;
static int current_thread_idx; // Index into all_threads about which is the currently running thread.
//static int all_threads_count = 0;
/**
 * Simple low level mutex locks
 */

static void _lock(volatile lowlevel_mutex_lock* lock) 
{
	// This function atomically checks if the value in *lock
	// is the same as the second parameter (0). If that is true
	// it writes the third parameter (1) to *lock, and returns true
    while (!__sync_bool_compare_and_swap ((int*)lock, 0, 1))
    {
        __asm__ __volatile__("rep; nop \n");
    }
}

static inline void _unlock(volatile lowlevel_mutex_lock * lock) {
	*lock = 0;
}

/**
 * Accessting thread data 
 * */
int __find_threadid_from_threaData(threadData * pth) {
	for(int  i = 0 ; i < NEEDLETHREAD_MAX_THREADS ; i++) {
		if(all_threads[i] == pth) {
			return i;
		}
	}
	return -1;
}

threadData * __extract_threadData_from_running_threads(int threadid) {
	// remember the pointer to what we are to take out of the list of running threads  
	threadData * toReturn = all_threads[threadid];

	all_threads[threadid] = 0;

	return toReturn;
}

int _find_next_thread_and_store(threadData * pnewThreadData) {
	// Optimize for success, allocate thread memory before locking.
	int found_idx=-1;

	_lock(&all_threads_lock);
	for(int i = 0 ; i < sizeof(all_threads)/ sizeof(all_threads[0]) ; i++) {
		if (all_threads[i] == 0) {
			found_idx = i;

			// Store it!
			all_threads[found_idx] = pnewThreadData; 
			break;
		}
	}
	_unlock(&all_threads_lock);

	return found_idx;
}



/** 
 * Functions used for the nitty gritty of switching between threads, i.e. stacks
 */
extern void __switch(int64_t ** storeSpHere, int64_t * useThisSp);
extern void __switch_abandon(int64_t ** ignored, int64_t * useThisSp);


int __yield(int abandon) {

	// TODO: These locks cant be held here, need to be managed by the switch
	// function. Technically the _unlock could be in the 
	_lock(&all_threads_lock);
	int leaving_thread_idx = current_thread_idx;

	// Look for next thread to run.
	// This needs serious optimization.
	do{
		current_thread_idx = (current_thread_idx + 1) % NEEDLETHREAD_MAX_THREADS;
	} while(all_threads[current_thread_idx] == 0);

	// Note to future implementors.
	// Any local variables that you create above this line....
	if(abandon) {
		threadData * pth = __extract_threadData_from_running_threads(leaving_thread_idx);
		free(pth->stack);
		free(pth);

		// AT THIS POINT WE NO LONGER HAVE A STACK, OR SHOULD ACCESS
		// VARIABLES FROM THE STACK. The function below will switch us
		// wherever we are going, and all parameters are global variables
		__switch_abandon( 0 , all_threads[current_thread_idx]->top_of_stack);

	} else {
		__switch(&all_threads[leaving_thread_idx]->top_of_stack, all_threads[current_thread_idx]->top_of_stack );
	}
	// ... may be different when you get below here. The Stack was swapped out by _switch*. 


	_unlock(&all_threads_lock);
}


void _init_and_register_main_thread() {
	threadData * pnewThreadData = malloc(sizeof(threadData));
	pnewThreadData->stack = 0; // Dont deallocate on exit

	_lock(&all_threads_lock);
	all_threads[0] = pnewThreadData;
	current_thread_idx = 0;
	_unlock(&all_threads_lock);

	return;
}



void __thread_root_function(void *(*pfunc)(void *), void* funcargs) {
	
	// TODO: Clean this up (The use of _unlock)
	// The reason for the stray unlock here is that the very first time
	// a function gets to run, it is not unfrozen because it returns
	// from a yield, but it just became alive. So in that case
	// the yield function that enabled it to run, has taken the lock. 
	// A thread that is already running for while would get reanimated 
	// returning through yield, thus unlocking, but not as a novel thread.
	_unlock(&all_threads_lock);

	//
	// Call the function that was passed by the caller to pthread_create
	void * retval = pfunc(funcargs);

	//
	// Terminate
	pthread_exit( retval );

	// Will never reach this
}


/**
 * Thread attributes
 */
int pthread_attr_init(pthread_attr_t * p)
{

	p->struct_state = pthread_attr_t_struct_state_POSTINIT;
	p->stack_size = NEEDLETHREAD_DEFAULT_STACK_SIZE; // TODO assert that its a good size.
	p->stack = 0;
	p->detached_state = PTHREAD_CREATE_JOINABLE;
	return 0;

}

int pthread_attr_destroy(pthread_attr_t * p) {

	if(p->struct_state == pthread_attr_t_struct_state_POSTINIT) {
		p->struct_state = pthread_attr_t_struct_state_POSTDESTROY;
		return 0;
	} else {
		return -1; // fail
	}
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int state) {
	if(state == PTHREAD_CREATE_DETACHED || state == PTHREAD_CREATE_JOINABLE) {
		attr->detached_state = state;
		return 0;
	} else {
		return -1;
	}
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *state){
	*state = attr->detached_state;
	return 0;
}

int   pthread_attr_setstacksize(pthread_attr_t * p, size_t s) {
	if(s < PTHREAD_STACK_MIN) {
		return EINVAL;
	} else {
		p->stack_size = s;
		return 0;
	}
}

int   pthread_attr_getstacksize(const pthread_attr_t * p, size_t * s) {
	*s = p->stack_size;
	return 0;
}


void pthread_exit(void * retval) {
	_lock(&all_threads_lock);
	threadData * me = all_threads[current_thread_idx];
	_unlock(&all_threads_lock);


	if(me->detached_state == PTHREAD_CREATE_JOINABLE) {
		// First, just hygiene store the exit state
		// and the fact that we are done.
		me->join_this_thread_is_done = 1; // Set to 1 in pthread_exit
		me->join_this_thread_retval = retval;  // Store return value passed in pthread_exit

		// Eventually the last joining thread 
		// will de-allocate our stack. Keep on 
		// Yielding until we die.
		while(1==1) {
			__yield(0); 
		}
	} else if(me->detached_state == PTHREAD_CREATE_DETACHED) {
		// woodo coding.
		// TODO implement __switch_abandon.
		__yield(1); // 1 = yield but discard the thread you are leaving.
	}
}

int pthread_join(pthread_t th, void ** retval){
	threadData * pth = (threadData *)th;
	
	threadData * me = all_threads[current_thread_idx];
	
	if(pth->detached_state == PTHREAD_CREATE_DETACHED) {
		return EINVAL;
	}

	if(pth->join_waited_on_by != 0) {
		return EINVAL;
	}

	if(pth == me) {
		return EDEADLK;
	}

	// TODO: When pthread_t is not a raw pointer to the thread data
	// we should also detect invalid handle and return ESRCH

	pth->join_waited_on_by = me;

	// Now, here is the algorithm.
	// Joining a thread means I will set a pointer in that threads data-structure
	// to point to me, and I will wait to be released. When the joined thread
	// releases me, i can then read the return value that the joined thread so 
	// kindly stored in my thread data.
	//-- Now, if MANY threads comes to join a thread, I will build a linked list,
	//-- and the joined thread will go through the list upon its exit and store
	//-- its return value in all of these, and release them.

	//-- Insert ourselves first in the list. 
	//--if(pth->join_waited_on_by != 0) {
	//--	me->join_others_waiting_too = pth->join_waited_on_by;
	//--	pth->join_waited_on_by = me;
	//--}

	// Force compiler to reload for every read.
	volatile int * exited = & pth->join_this_thread_is_done;
	while(!(*exited)) {
		__yield(0);
	}
	
	
	// Store the return value to the callers delight
	if(retval) {
		*retval = pth->join_this_thread_retval;
	}

	// Clean up the stack and other data for the thread that just died


	// TODO: Doh, this is stupid, need to fix this.
	_lock(&all_threads_lock);

	// remove the thread data from the subsystem, making it free to delete etc.
	__extract_threadData_from_running_threads(current_thread_idx); // returns pth again.
	
	_unlock(&all_threads_lock);

	free(pth->stack);
	free(pth);

	return 0;
}

int pthread_create(pthread_t *pth, const pthread_attr_t *pattr, void *(*pfunc)(void *) , void * funcargs) {

	const pthread_attr_t * attr_to_use;

	// If users did not pass any attributes create a default set.
	pthread_attr_t a;
	if(pattr == 0) {
		pthread_attr_init(&a);
		attr_to_use = &a;
	} else {
		attr_to_use = pattr;
	}

	int toReturn = 0;
	threadData * p_new_thread_data = malloc(sizeof(threadData));
	int64_t * p_new_stack = malloc(attr_to_use->stack_size);
	if(!p_new_thread_data || !p_new_stack) {
		// Could not allocate
		free(p_new_thread_data);
		free(p_new_stack);
		toReturn = EAGAIN;
	} else {
		// We are not done yet, clearly
		p_new_thread_data->join_this_thread_is_done = 0;
		p_new_thread_data->join_this_thread_retval = (void*)0xdeadbeefdeadbeef;
		p_new_thread_data->detached_state = attr_to_use->detached_state;

		p_new_thread_data->stack         = p_new_stack;
		//p_new_thread_data->stack_size    = attr_to_use->stack_size;
		p_new_thread_data->top_of_stack  = &p_new_thread_data->stack[(attr_to_use->stack_size  / sizeof(int64_t))  -1];

		// Set up the stack. This will be popped by the __swap function, as the 
		// running thread yields to the thread we are currently setting up.
		// The last thing that will be popped, is on top of this list, is the
		// adress that will go into the Instruction Pointer. This is done by
		// the "ret" instruction at the end of __swap.
		*(p_new_thread_data->top_of_stack-0) = (int64_t) __thread_root_function; // POPs to SP
		*(p_new_thread_data->top_of_stack-1) = (int64_t) &p_new_thread_data->stack[(attr_to_use->stack_size  / sizeof(int64_t))  -1];//RBP, the base of the stack essentially
		*(p_new_thread_data->top_of_stack-2) = 1;//push rax
		*(p_new_thread_data->top_of_stack-3) = 2;//push rbx
		*(p_new_thread_data->top_of_stack-4) = 3;//push rcx
		*(p_new_thread_data->top_of_stack-5) = 4;//push rdx
		*(p_new_thread_data->top_of_stack-6) = (int64_t) (p_new_thread_data->top_of_stack-1);//push rbp. Need this so the LEAVE function pops -1

		*(p_new_thread_data->top_of_stack-7) = (int64_t) funcargs;//push rsi - Second parameter to the C function
		*(p_new_thread_data->top_of_stack-8) = (int64_t) pfunc;//push rdi - First parameter to the C function
		*(p_new_thread_data->top_of_stack-9) = 8;//push r8
		*(p_new_thread_data->top_of_stack-10) = 9;//push r9
		*(p_new_thread_data->top_of_stack-11) = 10;//push r10
		*(p_new_thread_data->top_of_stack-12) = 11;//push r11
		*(p_new_thread_data->top_of_stack-13) = 12;//push r12
		*(p_new_thread_data->top_of_stack-14) = 13;//push r13
		*(p_new_thread_data->top_of_stack-15) = 14;//push r14
		*(p_new_thread_data->top_of_stack-16) = 15;//push r15
		p_new_thread_data->top_of_stack -= 16;

	
	}

	// TODO better error code.
	_find_next_thread_and_store(p_new_thread_data);

	*pth = (pthread_t) p_new_thread_data;

	if(pattr == 0) {
		pthread_attr_destroy(&a);
	}

	return toReturn;
}

