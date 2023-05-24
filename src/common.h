#ifndef COMMON_H
#define COMMON_H

#include "error_handling_utils.h"
#include "tsqueue.h"
#include "pthread_utils.h"
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <limits.h>

#define SOK_NAME "./soket634318"
#define SUN_MAX_LEN 108
#define STOP_STR "stop"

typedef struct data {
    size_t n;
    double avg;
    double std;
    char path[PATH_MAX];
} data_t;

#ifdef DEBUG
#define DPRINT(...) fprintf(stderr, __VA_ARGS__);
#else
#define DPRINT(...) ;
#endif


#endif
