#include "pthread.h"
#define _BITS_PTHREADTYPES_COMMON_H
#include <stdio.h>


// Make sure we did not pick up the OS pthread.h
#ifndef __PTHREAD_NEEDLETHREAD_H__
#error "Wrong pthread included"
#endif

extern void* call_on_new_stack(void* (*fun_ptr)(void*), int * stack_bottom, void* param);

int fakestack[10000];

void* printSomething(void*p) {
	printf("%s",(char*)p);
}

int main(int argc, const char ** argv) {

	printf("sizeof(int) = %lu",sizeof(int));
	//
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_destroy(&attr);

	printf("Done\n");

	call_on_new_stack(printSomething,&fakestack[10000-8],"Hello thread 1\n");
	call_on_new_stack(printSomething,&fakestack[10000-8],"Hello thread 2\n");

	printf("Done\n");

}