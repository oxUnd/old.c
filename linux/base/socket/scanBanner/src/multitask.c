#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "multitask.h"

int multi_task_create_by_pthread(pthread_t *  ntid, void * (*start_rtn) (void *), void *  arg) {
    int err;
    err = pthread_create(ntid, NULL, start_rtn, arg);
    if (err != 0) {
        fprintf(stderr, "Can't create thread: %s\n", strerror(err));
        exit(1);
    }
    return multi_task_get_selfid_by_pthread();
}

int multi_task_get_selfid_by_pthread(void) {
    return pthread_self();
}
