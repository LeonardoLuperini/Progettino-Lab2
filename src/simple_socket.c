#include "simple_socket.h"

struct sockaddr_un *sa_un_init(char *skt_name) {
    struct sockaddr_un *sa = malloc(sizeof(struct sockaddr_un));
    ERR_RET(sa == NULL, NULL);
    memset((void *)sa, 0, sizeof(struct sockaddr_un));

    strncpy(sa->sun_path, skt_name, SUN_MAX_LEN);
    sa->sun_family = AF_UNIX;

    return sa;
}

struct sockaddr_in *sa_in_init(char *address, uint16_t port) {
    struct sockaddr_in *sa = malloc(sizeof(struct sockaddr_in));
    ERR_RET(sa == NULL, NULL);
    memset((void *)sa, 0, sizeof(struct sockaddr_in));

    sa->sin_family = AF_INET;
    sa->sin_port = htons(port);
    ERR_PRINT_EXIT(inet_pton(AF_INET, address, &sa->sin_addr) == 0,
                   "Error inet_pton: Invalid address\n");

    return sa;
}

inline int socket_for_sa(struct sockaddr *sa, int type, int protocol) {
    return socket(sa->sa_family, type, protocol);
}

void simple_connect(int skt, const struct sockaddr *sa) {
    size_t refused_counter = 0;
    while (connect(skt, (struct sockaddr *)sa, sizeof(*sa)) == -1) {
        if (errno == ECONNREFUSED && refused_counter < 25) {
            refused_counter++;
            errno = 0;
            sleep(1);
            continue;
        }
        ERR_PERROR_EXIT(errno != ENOENT, "Error connect");
        sleep(1);
        errno = 0;
    }
}
