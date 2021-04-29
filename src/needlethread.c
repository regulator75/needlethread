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
	const char * stack;
} threadData;

typedef int lowlevel_mutex_lock;

/** 
 * Local data
 */


static threadData* all_threads[NEEDLETHREAD_MAX_THREADS] = {0};
static lowlevel_mutex_lock all_threads_lock = 0;


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


threadData * _find_next_thread_and_allocate() {
	// Optimize for success, allocate thread memory before locking.
	threadData * pnewThreadData = malloc(sizeof(threadData));
	int found_idx=-1;

	// Early exit if malloc fails
	if(!pnewThreadData) {
		return 0;
	}

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


	if(found_idx == -1) {
		// Too many threads allocated, free the threadMemory we prematurely allocated
		free(pnewThreadData);
		pnewThreadData = 0;
	}
	return pnewThreadData;
}

int pthread_attr_init(pthread_attr_t * p)
{

	p->struct_state = pthread_attr_t_struct_state_POSTINIT;
	p->stack_size = NEEDLETHREAD_DEFAULT_STACK_SIZE;
	p->stack = 0;
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


int pthread_create(pthread_t *pth, const pthread_attr_t *pattr, void *(*pfunc)(void *) , void * funcargs) {

	// Todo default thread attributes if its null.

	int toReturn = 0;
	threadData * p_new_thread_data = _find_next_thread_and_allocate();
	char * p_new_stack = malloc(pattr->stack_size);
	if(!p_new_thread_data || p_new_stack) {
		// Could not allocate
		free(p_new_thread_data);
		free(p_new_stack);
		toReturn = EAGAIN;
	} else {
		p_new_thread_data->stack = p_new_stack;

	}
	return toReturn;
}