#ifndef __MULTITASK_H__
#define __MULTITASK_H__

#include <pthread.h>

int multi_task_create_by_pthread(pthread_t * ntid, void * (*start_rtn) (void *), void * arg);
int multi_task_get_selfid_by_pthread(void);

#endif
