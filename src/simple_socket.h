#ifndef SIMPLE_SOCKET_H
#define SIMPLE_SOCKET_H

#include "error_handling_utils.h"
#include "debug_utils.h"
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SUN_MAX_LEN 108

struct sockaddr_un *sa_un_init(char *skt_name);
struct sockaddr_in *sa_in_init(char *address, uint16_t port);
int socket_for_sa(struct sockaddr *sa, int type, int protocol);
void simple_connect(int skt, const struct sockaddr* address);

#endif
