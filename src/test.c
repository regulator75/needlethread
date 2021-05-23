#include "pthread.h"
#define _BITS_PTHREADTYPES_COMMON_H
#include <stdio.h>


#define NEEDLE_TEST_ASSERT(x) if(!(x)) { printf("FAIL test "#x);} 

// Make sure we did not pick up the OS pthread.h
#ifndef __PTHREAD_NEEDLETHREAD_H__
#error "Wrong pthread included"
#endif

extern void* call_on_new_stack(void* (*fun_ptr)(void*), int * stack_bottom, void* param);
extern void _init_and_register_main_thread();
extern void __yield(int);

int fakestack[10000];

void* printSomething(void*p) {
	for(int i = 0 ; i < 10 ; i++) {
		printf("%s",(char*)p);
		__yield(0);
	};
}

void test0() {
	call_on_new_stack(printSomething,&fakestack[10000-8],"Hello stack 1\n");
	call_on_new_stack(printSomething,&fakestack[10000-8],"Hello stack 2\n");
}

void test1() {
	pthread_attr_t attr;
	size_t s;

	// Test init and destroy
	pthread_attr_init(&attr);
	pthread_attr_destroy(&attr);

	// Test pthread_attr_setstacksize. pthread_attr_getstacksize
	pthread_attr_init(&attr);
	NEEDLE_TEST_ASSERT(PTHREAD_STACK_MIN > 256); // random low number

	pthread_attr_getstacksize(&attr,&s);
	NEEDLE_TEST_ASSERT(s >= PTHREAD_STACK_MIN);
	
	NEEDLE_TEST_ASSERT(pthread_attr_setstacksize(&attr,PTHREAD_STACK_MIN-1) != 0);
	
	NEEDLE_TEST_ASSERT(pthread_attr_setstacksize(&attr,PTHREAD_STACK_MIN) == 0);
	
	pthread_attr_setstacksize(&attr,PTHREAD_STACK_MIN+1234);
	pthread_attr_getstacksize(&attr,&s);
	NEEDLE_TEST_ASSERT(s == PTHREAD_STACK_MIN+1234);


}

void test2() {

	pthread_t  th;
	pthread_create(&th,0,printSomething,"1\n");
	pthread_create(&th,0,printSomething," 2\n");
	pthread_create(&th,0,printSomething,"  3\n");	

	for(int i = 0 ; i < 100; i++) {
		__yield(0);
	}
}

void* sayBlahCount(void*pp) {
	long int p = (long int)pp;
	for(int i = p-1 ; i >= 0 ; i--) {
		printf("Blah Blah %d\n",i);
		__yield(0);
	};
	return (void*)(p*2);
}

void test3() {
	pthread_t  th;
	long int result;
	pthread_create(&th,0,sayBlahCount,(void*)5);
	__yield(0);
	__yield(0);
	printf("Joining\n");	
	pthread_join(th, (void*)&result);
	printf("Done joining, result = %lu\n", result);
}

void test4() {
	pthread_t th;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	
	pthread_create(&th,&attr,printSomething,"Detached\n");

	pthread_attr_destroy(&attr);	

	for(int i = 0 ; i < 20 ; i++) {
		__yield(0);
	}
}


int main(int argc, const char ** argv) {

	printf("sizeof(int) = %lu, sizeof(void*) = %lu\n",sizeof(int), sizeof(void*));
	_init_and_register_main_thread();
	
	//
	//test0();
	//test1();
	//test2();
	//test3();
	test4();

	printf("Done\n");
}