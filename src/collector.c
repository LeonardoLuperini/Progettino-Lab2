#include "common.h"
#include <poll.h>

#define NPRINTF(s, n)                                                          \
    for (int i = 0; i < n; i++)                                                \
        printf("%s", s);                                                       \
    printf("\n");

#define PRINT_HEADER()                                                         \
    printf("%-10s %-10s %-10s %-20s\n", "n", "avg", "std", "file");            \
    NPRINTF("-", 50);

#define NTHREADS 12

typedef struct arg {
    queue_t *queue;
    tscounter_t *c;
} arg_t;

static void *thread(void *arg);

static const int stop = -2;

int main(void) {
    int fd_listen_skt;
    int *fd_c;
    pthread_t tids[NTHREADS];
    arg_t arg;
    int listen_res;
    queue_t *clients = queue_init();
    tscounter_t *c = counter_init(0);
    ERR_PRINT_EXIT(clients == NULL, "Error queue_init\n");

    DPRINT("hello!\n");

    fd_listen_skt = socket(AF_UNIX, SOCK_STREAM, 0);
    ERR_PERROR_EXIT(fd_listen_skt == -1, "Error socket");

    struct sockaddr_un sa;
    strncpy(sa.sun_path, SOK_NAME, SUN_MAX_LEN);
    sa.sun_family = AF_UNIX;

    bind(fd_listen_skt, (struct sockaddr *)&sa, sizeof(sa));

    arg.queue = clients;
    arg.c = c;
    for (size_t i = 0; i < NTHREADS; i++) {
        sthread_create(&tids[i], thread, (void *)&arg);
    }

    struct pollfd poll_fd;
    int poll_res;
    poll_fd.fd = fd_listen_skt;
    poll_fd.events = POLLIN | POLLPRI;
    poll_fd.revents = 0;
    errno = 0;
    while (true) {
        poll_res = poll(&poll_fd, 1, 1000);
        if (poll_res == 0) {
            mtx_lock(&c->mtx);
            if (c->val == NTHREADS)
                break;
            mtx_unlock(&c->mtx);
        } else if (poll_res == 1) {
            listen_res = listen(fd_listen_skt, SOMAXCONN);
            ERR_PERROR_EXIT(listen_res != 0, "Error listen");

            fd_c = malloc(sizeof(int));
            ERR_PRINT_EXIT(fd_c == NULL, "Error malloc\n");
            *fd_c = accept(fd_listen_skt, NULL, 0);
            ERR_PERROR_EXIT(*fd_c == -1, "Error accept");
            ERR_PERROR_EXIT(!queue_push(clients, (void *)fd_c),
                            "Error queue_push");
        } else if (poll_res == -1) {
            ERR_PERROR_EXIT(errno == EINVAL, "Error poll");
            errno = 0;
        }
    }

    for (size_t i = 0; i < NTHREADS; i++) {
        sthread_join(tids[i], NULL);
    }

    close(fd_listen_skt);
    unlink(SOK_NAME);

    return EXIT_SUCCESS;
}

void *thread(void *arg) {
    data_t data;
    arg_t *a = (arg_t *)arg;
    int *fd_c;
    ssize_t readres;

    while ((fd_c = queue_pop(a->queue))) {
        if (*fd_c == stop) {
            a->c->val++;
            queue_push(a->queue, (void *)&stop);
            return NULL;
        }

        while ((readres = read(*fd_c, &data, sizeof(data_t))) > 0) {
            if (data.n == 0 && data.avg == -1 && data.std == -1 &&
                strcmp(data.path, STOP_STR) == 0) {
                DPRINT("HELLO\n");
                queue_push(a->queue, (void *)&stop);
                break;
            }
            printf("%lu\t%lf\t%lf\t%s\n", data.n, data.avg, data.std,
                   data.path);
        }
        if (readres == -1) {
            queue_push(a->queue, fd_c);
            perror("Error read");
        }
        close(*fd_c);
    }

    return NULL;
}
