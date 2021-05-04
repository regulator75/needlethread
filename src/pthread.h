#ifndef __PTHREAD_NEEDLETHREAD_H__
#define __PTHREAD_NEEDLETHREAD_H__

#include <stddef.h>
 

#define PTHREAD_CANCEL_ASYNCHRONOUS 101
#define PTHREAD_CANCEL_DEFERRED     102
#define PTHREAD_CANCEL_DISABLE      103
#define PTHREAD_CANCEL_ENABLE       104
#define PTHREAD_CANCELED            105


#define PTHREAD_CREATE_DETACHED     201
#define PTHREAD_CREATE_JOINABLE     202

#define PTHREAD_COND_INITIALIZER    301

#define PTHREAD_EXPLICIT_SCHED      401
#define PTHREAD_INHERIT_SCHED       402

#define PTHREAD_MUTEX_ERRORCHECK    501
#define PTHREAD_MUTEX_DEFAULT       502
#define PTHREAD_MUTEX_INITIALIZER   503
#define PTHREAD_MUTEX_NORMAL        504
#define PTHREAD_MUTEX_RECURSIVE     505

#define PTHREAD_ONCE_INIT           600

#define PTHREAD_PRIO_INHERIT        700
#define PTHREAD_PRIO_NONE           701
#define PTHREAD_PRIO_PROTECT        702

#define PTHREAD_PROCESS_SHARED      801
#define PTHREAD_PROCESS_PRIVATE     802

#define PTHREAD_RWLOCK_INITIALIZER  901

#define PTHREAD_SCOPE_PROCESS      1001
#define PTHREAD_SCOPE_SYSTEM       1002


/**
 * Constants
 */
#ifndef NEEDLETHREAD_DEFAULT_STACK_SIZE
#define NEEDLETHREAD_DEFAULT_STACK_SIZE 100000
#endif

#ifndef NEEDLETHREAD_MAX_THREADS
#define NEEDLETHREAD_MAX_THREADS 100
#endif


void Freeze();
/** 
 * Types
 */

typedef int pthread_t;
typedef int pthread_key_t;

typedef struct _tagpthread_cond_t {

} pthread_cond_t;

typedef struct _tagpthread_condattr_t {

} pthread_condattr_t;

typedef struct _tagpthread_mutex_t {

} pthread_mutex_t;

#define pthread_attr_t_struct_state_RAW 0
#define pthread_attr_t_struct_state_POSTINIT 1
#define pthread_attr_t_struct_state_POSTDESTROY 2

typedef struct _tagpthread_attr_t {
	int struct_state; // 0/random per default. 1 after init. 2 after destroy
	int stack_size;
	const char * stack; // Created on pthread_create, not on attr_init

	int detached_state; //default PTHREAD_CREATE_JOINABLE

} pthread_attr_t;

typedef struct _tagpthread_rwlock_t {

} pthread_rwlock_t;

typedef struct _tagpthread_rwlockattr_t {

} pthread_rwlockattr_t; 

typedef struct _tagpthread_mutexattr_t {

} pthread_mutexattr_t;

typedef struct _tagpthread_once_t {

} pthread_once_t; 

// From #include <sched.h>
typedef struct _tagsched_param {

} sched_param;

// From #include <time.h>
typedef struct _tagptimespec {

} timespec;

/** 
 * General Pthread attribute management 
 */
int   pthread_attr_init(pthread_attr_t *);
int   pthread_attr_destroy(pthread_attr_t *);

int   pthread_attr_getdetachstate(const pthread_attr_t *, int *);
int   pthread_attr_setdetachstate(      pthread_attr_t *, int);

int   pthread_attr_setguardsize(pthread_attr_t *, size_t);
int   pthread_attr_getguardsize(const pthread_attr_t *, size_t *);

int   pthread_attr_setinheritsched(pthread_attr_t *, int);
int   pthread_attr_getinheritsched(const pthread_attr_t *, int *);

int   pthread_attr_setschedparam(pthread_attr_t *,const sched_param *);
int   pthread_attr_getschedparam(const pthread_attr_t *, sched_param *);

int   pthread_attr_setschedpolicy(pthread_attr_t *, int);
int   pthread_attr_getschedpolicy(const pthread_attr_t *, int *);

int   pthread_attr_setscope(pthread_attr_t *, int);
int   pthread_attr_getscope(const pthread_attr_t *, int *);

int   pthread_attr_setstackaddr(pthread_attr_t *, void *);
int   pthread_attr_getstackaddr(const pthread_attr_t *, void **);

int   pthread_attr_setstacksize(pthread_attr_t *, size_t);
int   pthread_attr_getstacksize(const pthread_attr_t *, size_t *);

int   pthread_cancel(pthread_t);

/** 
 * Cleanup routine management 
 */
void  pthread_cleanup_push(void (*cleanup_routine)(void*), void *arg);
void  pthread_cleanup_pop(int);

/**
 * Condition variable management, including their attributes
 */

int   pthread_cond_init(pthread_cond_t *, const pthread_condattr_t *);
int   pthread_cond_destroy(pthread_cond_t *);

int   pthread_cond_broadcast(pthread_cond_t *);
int   pthread_cond_signal(pthread_cond_t *);
int   pthread_cond_timedwait(pthread_cond_t *,pthread_mutex_t *, const timespec *);
int   pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *);

int   pthread_condattr_init(pthread_condattr_t *);
int   pthread_condattr_destroy(pthread_condattr_t *);

int   pthread_condattr_setpshared(pthread_condattr_t *, int);
int   pthread_condattr_getpshared(const pthread_condattr_t *, int *);

/**
 * Creation and actual use of threads.
 */
int   pthread_create(pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);
int   pthread_join(pthread_t, void **);

int   pthread_detach(pthread_t);
int   pthread_equal(pthread_t, pthread_t);
int   pthread_setconcurrency(int);

int   pthread_getconcurrency(void);

int   pthread_setschedparam(pthread_t, int , const sched_param *);
int   pthread_getschedparam(pthread_t, int *, sched_param *);

int   pthread_setspecific(pthread_key_t, const void *);
void *pthread_getspecific(pthread_key_t);
int   pthread_key_create(pthread_key_t *, void (*)(void *));
int   pthread_key_delete(pthread_key_t);

int   pthread_setcanceltype(int, int *);
int   pthread_setcancelstate(int, int *);

// Functions to operate on thy self.
pthread_t pthread_self(void);
void  pthread_exit(void *);


/**
 * Mutexes and their attributes
 */

int   pthread_mutex_init(pthread_mutex_t *, const pthread_mutexattr_t *);
int   pthread_mutex_destroy(pthread_mutex_t *);

int   pthread_mutex_setprioceiling(pthread_mutex_t *, int, int *);
int   pthread_mutex_getprioceiling(const pthread_mutex_t *, int *);

int   pthread_mutex_lock(pthread_mutex_t *);
int   pthread_mutex_trylock(pthread_mutex_t *);
int   pthread_mutex_unlock(pthread_mutex_t *);

int   pthread_mutexattr_init(pthread_mutexattr_t *);
int   pthread_mutexattr_destroy(pthread_mutexattr_t *);
int   pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *, int *);
int   pthread_mutexattr_getprotocol(const pthread_mutexattr_t *, int *);
int   pthread_mutexattr_getpshared(const pthread_mutexattr_t *, int *);
int   pthread_mutexattr_gettype(const pthread_mutexattr_t *, int *);
int   pthread_mutexattr_setprioceiling(pthread_mutexattr_t *, int);
int   pthread_mutexattr_setprotocol(pthread_mutexattr_t *, int);
int   pthread_mutexattr_setpshared(pthread_mutexattr_t *, int);
int   pthread_mutexattr_settype(pthread_mutexattr_t *, int);

int   pthread_once(pthread_once_t *, void (*)(void));

int   pthread_rwlock_init(pthread_rwlock_t *, const pthread_rwlockattr_t *);
int   pthread_rwlock_destroy(pthread_rwlock_t *);

int   pthread_rwlock_rdlock(pthread_rwlock_t *);
int   pthread_rwlock_tryrdlock(pthread_rwlock_t *);
int   pthread_rwlock_trywrlock(pthread_rwlock_t *);
int   pthread_rwlock_unlock(pthread_rwlock_t *);
int   pthread_rwlock_wrlock(pthread_rwlock_t *);


int   pthread_rwlockattr_init(pthread_rwlockattr_t *);
int   pthread_rwlockattr_destroy(pthread_rwlockattr_t *);

int   pthread_rwlockattr_setpshared(pthread_rwlockattr_t *, int);
int   pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *, int *);






#endif //__PTHREAD_NEEDLETHREAD_H__
