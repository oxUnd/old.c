#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

typedef struct arg arg_t;
struct arg {
    int a;
    int b;
};

typedef struct task task_t;
typedef struct task *task_ptr;
struct task {
    void * (* callback_proc) (void *);
    void *arg;
    task_t * next;
};

typedef struct pool pool_t;
typedef struct pool * pool_ptr;
struct pool {
    pthread_t *threads;
    u_int8_t n_max_threads;
    pthread_mutex_t p_lock;
    pthread_cond_t p_ready;
    int n_tasks;
    task_t *task_set;
};

pool_t * g_pool_set;
int g_showdown_all = 0;

void err_quit(const char *msg);
pool_t * pool_init(u_int8_t n_max_threads);
int pool_destroy(pool_t *pool_set);
int pool_add_task(pool_t *pool_set, task_t * task);

void * process_routine(void *arg);

task_t * task_init(void);
int task_add(task_ptr * task_set, task_t *task);
int task_destroy(task_t *task_set);

void * proc(void * arg);

void err_quit(const char *msg) {
    perror(msg);
    exit(1);
}

void * process_routine(void *arg) {
    printf ("starting thread 0x%x\n", (unsigned int)pthread_self ());
    for (;;) {
        pthread_mutex_lock(&(g_pool_set->p_lock));
        while (g_pool_set->n_tasks == 0 && g_showdown_all == 0) { //线程阻塞
            printf ("thread 0x%x is waiting\n", (unsigned int)pthread_self ());
            pthread_cond_wait(&(g_pool_set->p_ready), &(g_pool_set)->p_lock);
        }
        if (g_showdown_all == 1) {
            pthread_mutex_unlock(&(g_pool_set->p_lock));
            pthread_exit(NULL); //线程退出
        }
        task_t *task = g_pool_set->task_set;
        if (task == NULL) continue;
        g_pool_set->task_set = task->next; //get head task
        g_pool_set->n_tasks--; //set task count, -1
        pthread_mutex_unlock(&(g_pool_set->p_lock));
        printf("callback addr 0x%x\n", (unsigned int) task->arg);
        *(task->callback_proc)(task->arg);

    }
}

task_t * task_init(void) {
    task_t *task_set = (task_t *)malloc(sizeof(task_t));
    task_set->callback_proc = NULL;
    task_set->arg = NULL;
    task_set->next = NULL;
    return task_set;
}

int task_add(task_ptr *task_set, task_t * task) {
    if (task == NULL)
        return 1;
    if (*task_set == NULL) {
        *task_set = task;
    } else {
        task_t *t = *task_set;
        while (t->next != NULL) t = t->next;
        t->next = task;
    }
    if (*task_set == NULL) return 1;
    assert(*task_set != NULL);
    return 0;
}

int task_destroy(task_t * task_set) {
    task_t * t = task_set;
    if (t != NULL) {
        t = task_set->next;
        task_set->arg = NULL;
        task_set->callback_proc = NULL;
        task_set = t;
    }
    return 0;
}

pool_t * pool_init(u_int8_t n_max_threads) {
    pthread_t *threads = (pthread_t *)malloc(n_max_threads * sizeof(pthread_t));
    if (threads == NULL) {
        err_quit("# [int] malloc failed");
    }
    pool_t * pool_set = (pool_t *) malloc(sizeof(pool_t));
    if (pool_set == NULL) err_quit("# [pool_t] malloc failed");

    pool_set->n_max_threads = n_max_threads;
    pool_set->task_set = NULL;

    pthread_mutex_init(&(pool_set->p_lock), NULL);
    pthread_cond_init(&(pool_set->p_ready), NULL);

    pool_set->n_tasks = 0; //线程等待条件

    g_pool_set = pool_set; //由于创建线程时需要互斥锁，所以必须现在设置g_pool_set

    int i;
    int err;
    for (i = 0; i < n_max_threads; ++i) {
        err = pthread_create(&(threads[i]), NULL, process_routine, NULL);
    }
    pool_set->threads = threads;
    return pool_set;
}

int pool_add_task(pool_t *pool_set, task_t *task) {
    if (pool_set == NULL || task == NULL) return 1;
    int ret;

    pthread_mutex_lock(&(g_pool_set->p_lock));
    ret = task_add(&(pool_set->task_set), task);
    if (ret == 0) {
        pool_set->n_tasks++;
        pthread_mutex_unlock(&(pool_set->p_lock));
        pthread_cond_signal(&(pool_set->p_ready));
        return 0;
    }
    pthread_mutex_unlock(&(g_pool_set->p_lock));
    return 1;
}

int pool_destroy(pool_t * pool_set) {
    if (pool_set != NULL) {
        if (g_showdown_all == 0) g_showdown_all = 1;
        int i = 0;
        pthread_cond_broadcast(&(pool_set->p_ready)); //唤醒等待的线程
        for (i = 0; i < pool_set->n_max_threads; ++i)
            pthread_join(pool_set->threads[i], NULL);
        free(pool_set->threads);
        pool_set->threads = NULL;
        pthread_mutex_destroy(&(pool_set->p_lock));
        pthread_cond_destroy(&(pool_set->p_ready));
        free(pool_set);
        pool_set = NULL;
    }
    return 0;
}

void * proc(void * arg) {
    int *n = (int *)arg;
    printf("runing thread 0x%x\n", (unsigned int) pthread_self());
    printf(">> %d\n", *n);
    sleep(1);
    return ((void *) 0);
}

int main() {
    u_int8_t n_thds = 3;
    g_pool_set = pool_init(n_thds);
    task_t * task = task_init();
    int * workingnum = (int *) malloc (sizeof (int) * 10);
    int i;


    for (i = 0; i < 10; i++) {
        task = task_init();
        task->callback_proc = &proc;
        workingnum[i] = i;
        //参数地址取不同地址，这个很必要，不然就会发生冲突
        task->arg = (void *) &workingnum[i];
        pool_add_task(g_pool_set, task);
    }
    sleep(6);
    pool_destroy(g_pool_set);
    free(workingnum);
    return 0;
}
