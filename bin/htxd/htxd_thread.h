/* @(#)70	1.2  src/htx/usr/lpp/htx/bin/htxd/htxd_thread.h, htxd, htxubuntu 9/2/14 09:03:34 */



#ifndef __HTXD_THREAD__
#define __HTXD_THREAD__


#include <pthread.h>

typedef struct{
	pthread_t thread_id;
	void *(*thread_function)(void *);
	void *thread_data;
	} htxd_thread;



extern int htxd_thread_create(htxd_thread *);
extern int htxd_thread_cancel(htxd_thread *);
extern int htxd_enable_thread_cancel_state_type(void);
extern int htxd_start_hang_monitor(htxd_thread **);
extern int htxd_stop_hang_monitor(htxd_thread **);



#endif
