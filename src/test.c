#include "pthread.h"
#define _BITS_PTHREADTYPES_COMMON_H
#include <stdio.h>


// Make sure we did not pick up the OS pthread.h
#ifndef __PTHREAD_NEEDLETHREAD_H__
#error "Wrong pthread included"
#endif

extern void* call_on_new_stack(void* (*fun_ptr)(void*), int * stack_bottom, void* param);
extern void _init_and_register_main_thread();
extern void __yield();

int fakestack[10000];

void* printSomething(void*p) {
	for(int i = 0 ; i < 10 ; i++) {
		printf("%s",(char*)p);
		__yield();
	};
}

void test0() {
	call_on_new_stack(printSomething,&fakestack[10000-8],"Hello stack 1\n");
	call_on_new_stack(printSomething,&fakestack[10000-8],"Hello stack 2\n");
}

void test1() {
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_destroy(&attr);	
}

void test2() {

	pthread_t  th;
	pthread_create(&th,0,printSomething,"1\n");
	pthread_create(&th,0,printSomething," 2\n");
	pthread_create(&th,0,printSomething,"  3\n");	

	for(int i = 0 ; i < 100; i++) {
		__yield();
	}
}

void* sayBlahCount(void*pp) {
	long int p = (long int)pp;
	for(int i = p-1 ; i >= 0 ; i--) {
		printf("Blah Blah %d\n",i);
		__yield();
	};
	return (void*)(p*2);
}

void test3() {
	pthread_t  th;
	long int result;
	pthread_create(&th,0,sayBlahCount,(void*)5);
	__yield();
	__yield();
	printf("Joining\n");	
	pthread_join(th, (void*)&result);
	printf("Done joining, result = %lu\n", result);
}
int main(int argc, const char ** argv) {

	printf("sizeof(int) = %lu, sizeof(void*) = %lu\n",sizeof(int), sizeof(void*));
	_init_and_register_main_thread();
	
	//
	//test0();
	test1();
	test2();
	test3();

	printf("Done\n");
}