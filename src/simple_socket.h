#ifndef SIMPLE_SOCKET_H
#define SIMPLE_SOCKET_H

#include "error_handling_utils.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SUN_MAX_LEN 108

struct sockaddr_un *sa_un_init(char *skt_name);


#endif
