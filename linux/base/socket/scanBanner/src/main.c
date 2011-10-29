#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>
#include "array.h"
#include "request.h"
#include "multitask.h"

#define STD_BUFF_SIZE 1024
#define B_W_SIZE 255

#define MULTI_TASK_SIZE 10

#define TIME_OUT 6 

typedef enum {FALSE, TRUE} bool;


typedef struct {
    char *re;
    char *orig;
    short is_set;
} filter;

void show_help(void) {
    const char *help =
"Usage: scanBanner options\n"
"-h: hostname\n"
"-p: port\n"
"-r: range, eg: 10.16.0.1-100\n";
    fprintf(stdout, "%s", help);
    exit(0);
}

char *reverse (char *s) {
    int i;
    int s_len = strlen(s);
    char ch;
    for (i = 0; i < s_len / 2; i++) {
        ch = *(s + i);
        *(s + i) = *(s + s_len - i - 1);
        *(s + s_len - i - 1) = ch;
    }
    return s;
}

array * filter_by_re(const char *orig, const char *pattern, array *match) {
    regex_t o_regex;
    int n_err_code = 0;
    int n_match;
    regmatch_t matchs[B_W_SIZE];
    int i;

    char s_errmsg[STD_BUFF_SIZE];
    memset(s_errmsg, '\0', STD_BUFF_SIZE);

    size_t err_msg_len = 0;

#ifdef DEBUG
    fprintf(stdout, "# Orig [%s] - Pattern [%s] #\n", orig, pattern);
#endif
    if ((n_err_code = regcomp(&o_regex, pattern, 0)) == 0) {
        if ((n_err_code != regexec(&o_regex, orig, n_match, matchs, 0))) {
            printf("# RE Match Error Code [%d] #\n", n_err_code);
        }
    }
    printf("# Match NUM [%d] #\n", n_match);

    for (i = 0; i < n_match && matchs[i].rm_so != -1; ++i) {
        printf("$%d='%s[%d-%d]'\n", i, orig, matchs[i].rm_so, matchs[i].rm_eo);
    }

    regfree(&o_regex);
    return match;
}

int parse_arguments_ip(array *a, const char *ips, const int ips_len) {
    data_string *ds;
    char buf[16] = "";
    char tmp[16] = "";
    int i, j = 0;
    short min, max;
    int flag = 1;
    for (i = ips_len - 1; i >= 0; --i) {
        buf[j] =  ips[i];
        if (ips[i] == '-') {
            buf[j] = '\0';
            max = atoi(reverse(buf));
            memset(buf, '\0', strlen(buf));
            j = 0;
            continue;
        } else if (flag && ips[i] == '.') {
            buf[j] = '\0';
            min = atoi(reverse(buf));
            memset(buf, '\0', strlen(buf));
            flag = 0;
            j = 0;
            continue;
        }
        ++j;
    }
    j = min;
    reverse((char *) buf);
#ifdef DEBUG
    fprintf(stdout, "# Scan IP Range: %s.%d-%d #\n", buf, min, max);
#endif
    while (j < max) {
        ds = data_string_init();
        sprintf(tmp, "%s.%d", buf, j);
        buffer_copy_string_len(ds->value, (const char *)tmp, strlen(tmp));
        sprintf(tmp, "%d", j);
        buffer_copy_string_len(ds->key, (const char *)tmp, strlen(tmp));
        array_insert_unique(a, (data_unset *)ds);
        ++j;
    }
}

typedef struct {
    char * host;
    unsigned int port;
    filter * flt;
} task_arg;

pthread_mutex_t *f_lock;

/**
 * callback func
 */
void * subtask (void * arg) {
    task_arg * ta = (task_arg *) arg;
    int clientfd;
    char buffer[STD_BUFF_SIZE] = "";
    char head[STD_BUFF_SIZE] = "";
    clientfd = open_clientfd(ta->host, ta->port, TIME_OUT);
    if (clientfd < 0) {
        pthread_exit((void *) 0);
        return ((void *) 0); 
    }

    memset(head, '\0', strlen(head));
    sprintf(head, "HEAD / HTTP/1.0\r\n\r\n");
    send(clientfd, head, strlen(head) + 1);

    memset(buffer, '\0', strlen(buffer));

    recv(clientfd, buffer, 2048, 0);

    pthread_mutex_lock(f_lock);
    fprintf(stdout, "running thread 0x%x\n", (unsigned int)pthread_self());
    fprintf(stdout, "hostname %s\n port %d\n", ta->host, ta->port);
    fprintf(stdout, "recv data(%d)\n ", strlen(buffer));
    if (ta->flt->is_set) {
        filter_by_re(buffer, ta->flt->re, NULL);
    } else {
        fprintf(stdout, "%s\n", buffer);
    }
    pthread_mutex_unlock(f_lock);

    close(clientfd);

    pthread_exit((void *) 0);
    return ((void *) 0); 
}

void * task (const array *arr, const unsigned int port, filter * flt) {

    int i;
    int task_i = 0;

    data_string *ds;

    task_arg  *ta = (task_arg *) malloc(sizeof(task_arg) * arr->used);
    if (ta == NULL) {
        perror("malloc failed");
        exit(1);
    }
#ifdef DEBUG
    fprintf(stderr, "# Task Arg 0x%x %d #\n", (unsigned int) arr, port);
#endif
    pthread_t tnid;
    pthread_mutex_t fl;

    pthread_mutex_init(&fl, NULL);

    f_lock = &fl;

    /**
     * invoking subtask
     */

    for (i = 0; i < arr->used; ++i) {
        ds = (data_string *) arr->data[i];
        ta[i].port = port;
        ta[i].flt = flt;
        ta[i].host = ds->value->ptr;
        multi_task_create_by_pthread(&tnid, subtask, (void *)&(ta[i])); 
    }

    pthread_join(tnid, NULL);
    sleep(TIME_OUT * 2);
    pthread_mutex_destroy(&fl);
    f_lock = NULL;
    free(ta); //delete task_arg
    ta = NULL;
    return ((void *) 0);
}

int main (int argc, char **argv) {
    char hostname[STD_BUFF_SIZE] = "";
    unsigned int port;
    char ch;
    data_string *ds;

    filter flt = {"", "", FALSE};
    
    array * arr = array_init();

    if (argc == 1) {
        show_help();
    }

    while((ch = getopt(argc, argv, "h:p:r:f:")) != -1) {
        switch (ch) {
            case 'h':
                ds = data_string_init();
                buffer_copy_string_len(ds->key, CONST_STR_LEN("HOSTNAME"));
                buffer_copy_string_len(ds->value, (const char *)optarg, strlen(optarg));
                array_insert_unique(arr, (data_unset *) ds);
                break;
            case 'p':
                port = (unsigned int) atoi((const char *) optarg);
                break;
            case 'r':
                //strcpy(hostname, optarg);
                parse_arguments_ip(arr, (const char *)optarg, strlen(optarg));
                break;
            case 'f':
                flt.re = optarg;
                flt.is_set = TRUE;
                break;
            default:
                show_help();
                break;
        }
    }

    task(arr, port, &flt);
    
    array_free(arr);
    return 0;
}
