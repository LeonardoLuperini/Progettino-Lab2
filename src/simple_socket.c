#include "simple_socket.h"

struct sockaddr_un *sa_un_init(char *skt_name) {
    struct sockaddr_un *sa = malloc(sizeof(struct sockaddr_un));
    ERR_RET(sa == NULL, NULL);
    memset((void *)sa, 0, sizeof(struct sockaddr_un));
    strncpy(sa->sun_path, skt_name, SUN_MAX_LEN);
    sa->sun_family = AF_UNIX;
    return sa;
}
