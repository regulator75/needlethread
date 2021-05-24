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

void cleanup_routine(void * arg) {
	printf("cleanup routine: %lu\n", (long int)arg);
}

void* cleanup_test_thread(void* p) {
	long test = (long)p;
	switch(test) 
	{
		case 0:
		// printout should be "1"
		pthread_cleanup_push(cleanup_routine,(void*)1);
		pthread_exit(0);
		// Should never happen
		printf("FAIL");
		pthread_cleanup_pop(0);
		break;

		case 1:
		// printout should be "10, 20"
		pthread_cleanup_push(cleanup_routine,(void*)3);
		pthread_cleanup_push(cleanup_routine,(void*)2);
		pthread_exit(0);
		// Should never happen
		printf("FAIL");
		pthread_cleanup_pop(0);

		case 2:
		// 
		printf("Between this...\n");
		pthread_cleanup_push(cleanup_routine,(void*)666);
		pthread_cleanup_push(cleanup_routine,(void*)1);
		pthread_cleanup_pop(0);
		pthread_cleanup_pop(0);
		printf("... and this there should be no text, and 666 should never appear\n");
		pthread_exit(0);

		case 3:
		// 
		printf("The number 888 should never be printed.\n");
		pthread_cleanup_push(cleanup_routine,(void*)888);
		pthread_cleanup_push(cleanup_routine,(void*)1);
		return 0; // Thread function return, 
		pthread_cleanup_pop(0); // compiler friendly for other pthread libraries
		pthread_cleanup_pop(0); // compiler friendly for other pthread libraries
	}
	return 0;
}

void test4() {
	pthread_t th;
	void* result;
	pthread_create(&th,0,cleanup_test_thread,(void*)0);
	pthread_join(th, (void*)&result);

	pthread_create(&th,0,cleanup_test_thread,(void*)1);
	pthread_join(th, (void*)&result);

	pthread_create(&th,0,cleanup_test_thread,(void*)2);
	pthread_join(th, (void*)&result);

	pthread_create(&th,0,cleanup_test_thread,(void*)3);
	pthread_join(th, (void*)&result);
}


void test5(){
	pthread_t  th;
	unsigned long int result;
	pthread_create(&th,0,cleanup_test_thread,0);
	__yield(0);
	__yield(0);
	printf("Joining\n");	
	pthread_join(th, (void*)&result);
	printf("Done joining, result = %lu\n", result);
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