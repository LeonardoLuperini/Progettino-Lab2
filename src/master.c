#include "common.h"
#include "files.h"
#include "simple_socket.h"
#include "statistic.h"
#include <dirent.h>

#define CORRECT_SINTAX_MSG(name)                                               \
    printf("The correct syntax is:\n"                                          \
           "\t%s  <n. threads> <path_to_file>\n",                              \
           name);

#define STOP 0

typedef struct arg {
    queue_t *files;
    struct sockaddr *sa;
    tscounter_t *counter;
    size_t nworker;
} arg_t;

static void get_cli_input(int argc, char **argv, size_t *nworker, char **path);
static bool str_is_size(char *str);
static void stop_server(int fd_skt, tscounter_t *c, size_t nworker);
static void search_datfiles(char *path, queue_t *files);
static void *worker(void *arg);
static void compute_and_send(int fd_skt, queue_t *files);

int main(int argc, char *argv[]) {
    size_t nworker;
    char *path;
    tscounter_t *c = counter_init(1);

    get_cli_input(argc, argv, &nworker, &path);
    pthread_t tids[nworker];
    arg_t arg[nworker];

    queue_t *files = queue_init();
    ERR_PRINT_EXIT(files == NULL, "Error queue_init\n");

    struct sockaddr_in *sa = sa_in_init(SKT_ADDRESS, PORT);

    for (size_t i = 0; i < nworker; i++) {
        arg[i].files = files;
        arg[i].sa = (struct sockaddr *)sa;
        arg[i].counter = c;
        arg[i].nworker = nworker;
        sthread_create(&tids[i], worker, (void *)&arg[i]);
        DPRINT("Created %d\n", arg[i].n);
    }

    search_datfiles(path, files);

    for (size_t i = 0; i < nworker; i++)
        ERR_PRINT_EXIT(!queue_push(files, STOP), "Error queue_push\n");

    for (size_t i = 0; i < nworker; i++)
        sthread_join(tids[i], NULL);

    free(sa);
    counter_del(c);
    queue_destroy(files);

    return EXIT_SUCCESS;
}

static void *worker(void *arg) {
    arg_t *arg1 = (arg_t *)arg;
    struct sockaddr *sa = arg1->sa;

    int fd_skt = socket_for_sa(sa, SOCK_STREAM, 0);
    ERR_PERROR_EXIT(fd_skt == -1, "Error socket");

    simple_connect(fd_skt, sa);

    compute_and_send(fd_skt, arg1->files);

    stop_server(fd_skt, arg1->counter, arg1->nworker);

    close(fd_skt);

    return EXIT_SUCCESS;
}

static void compute_and_send(int fd_skt, queue_t *files) {
    double *num_array = NULL;
    size_t num_array_len = 0;
    char *path;
    data_t data;
    memset((void *)&data, 0, sizeof(data));
    data.n = 0;
    path = (char *)queue_pop(files);
    while (path != STOP) {
        file_to_array(path, &num_array, &num_array_len, &data.n);
        data.avg = avg(num_array, data.n);
        data.std = std(num_array, data.n);
        strcpy(data.path, path);
        write(fd_skt, &data, sizeof(data_t));
        free(path);
        path = (char *)queue_pop(files);
    }
    free(num_array);
}

static void stop_server(int fd_skt, tscounter_t *c, size_t nworker) {
    mtx_lock(&c->mtx);
    if (c->val < nworker) {
        (c->val)++;
        mtx_unlock(&c->mtx);
    } else {
        data_t data;
        memset((void *)&data, 0, sizeof(data));
        data.n = 0;
        data.std = -1;
        data.avg = -1;
        strcpy(data.path, STOP_STR);
        write(fd_skt, &data, sizeof(data_t));
        mtx_unlock(&c->mtx);
    }
}

static void search_datfiles(char *path, queue_t *files) {
    char *newpath;
    struct dirent *direlem;
    char *fname;
    size_t len_fname;
    char *fext;

    DIR *dir = opendir(path);
    ERR_PERROR_EXIT(dir == NULL, "Error opendir");
    while (errno == 0 && (direlem = readdir(dir))) {
        if (IS_SUB_DIR(direlem)) {
            newpath = malloc(sizeof(char) * PATH_MAX);
            ERR_PERROR_EXIT(newpath == NULL, "Error malloc");

            concat_path(path, direlem->d_name, newpath);
            search_datfiles(newpath, files);

            free(newpath);
        }
        if IS_FILE (direlem) {
            fname = direlem->d_name;
            len_fname = strlen(fname);
            fext = fname + len_fname - 4;
            if (strcmp(fext, ".dat") == 0) {
                newpath = malloc(sizeof(char) * PATH_MAX);
                ERR_PRINT_EXIT(newpath == NULL, "Error malloc\n");
                concat_path(path, fname, newpath);
                ERR_PRINT_EXIT(!queue_push(files, (void *)newpath),
                               "Error queue_push");
            }
        }
        errno = 0;
    }
    ERR_PERROR_EXIT(errno != 0 && direlem == NULL, "Error readdir")
    closedir(dir);
}

static bool str_is_size(char *str) {
    while (*str) {
        if (!isdigit(*str))
            return false;
        str++;
    }
    return true;
}

static void get_cli_input(int argc, char **argv, size_t *nworker, char **path) {
    if (argc != 3) {
        CORRECT_SINTAX_MSG(argv[0]);
        exit(EXIT_FAILURE);
    }

    // n. of worker
    if (!str_is_size(argv[1])) {
        fprintf(stderr, "%s is not a valid quantity!\n", argv[1]);
        CORRECT_SINTAX_MSG(argv[0]);
        exit(EXIT_FAILURE);
    }

    sscanf(argv[1], "%lu", nworker);
    if (*nworker == 0) {
        fprintf(stderr, "%s is not a valid quantity!\n", argv[1]);
        CORRECT_SINTAX_MSG(argv[0]);
        exit(EXIT_FAILURE);
    }

    // file path
    *path = argv[2];
}
