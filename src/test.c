#include "pthread.h"
#define _BITS_PTHREADTYPES_COMMON_H
#include <stdio.h>


// Make sure we did not pick up the OS pthread.h
#ifndef __PTHREAD_NEEDLETHREAD_H__
#error "Wrong pthread included"
#endif



int main(int argc, const char ** argv) {

	//
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_destroy(&attr);

	printf("Done\n");
}