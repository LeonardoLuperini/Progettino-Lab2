#include "common.h"
#include "files.h"
#include "statistic.h"
#include <dirent.h>

#define CORRECT_SINTAX_MSG(name)                                               \
    printf("The correct syntax is:\n"                                          \
           "\t%s  <n. threads> <path_to_file>\n",                              \
           name);

#define STOP 0

typedef struct arg {
    queue_t *files;
    struct sockaddr_un *sa;
    tscounter_t *counter;
    size_t nworker;
#ifdef DEBUG
    unsigned int n;
#endif
} arg_t;

static void get_cli_input(int argc, char **argv, size_t *nworker, char **path);
static bool str_is_size(char *str);
static void search_datfiles(char *path, queue_t *files);
static void *worker(void *arg);

int main(int argc, char *argv[]) {
    size_t nworker;
    char *path;
    tscounter_t *c = counter_init(1);

    get_cli_input(argc, argv, &nworker, &path);
    // DPRINT("nworker: %lu\npath: %s\n", nworker, path);
    pthread_t tids[nworker];
    arg_t arg[nworker];

    queue_t *files = queue_init();
    ERR_PRINT_EXIT(files == NULL, "Error queue_init\n");

    struct sockaddr_un *sa = sa_un_init(SOK_NAME);

    for (size_t i = 0; i < nworker; i++) {
        arg[i].files = files;
        arg[i].sa = sa;
        arg[i].counter = c;
        arg[i].nworker = nworker;
#ifdef DEBUG
        arg[i].n = i;
#endif
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
    unlink(SOK_NAME);

    return EXIT_SUCCESS;
}

static void *worker(void *arg) {
    arg_t *arg1 = (arg_t *)arg;
    struct sockaddr_un *sa = (struct sockaddr_un *)arg1->sa;
    queue_t *files = (queue_t *)arg1->files;
    tscounter_t *c = arg1->counter;
#ifdef DEBUG
    unsigned int n = arg1->n;
#endif

    int fd_skt;

    DPRINT("%u: Hello\n", n);

    // Soket
    fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);
    ERR_PERROR_EXIT(fd_skt == -1, "Error socket");

    DPRINT("%u: try to connect to %s\n", n, SOK_NAME);
    size_t refused_counter = 0;
    while (connect(fd_skt, (struct sockaddr *)sa, sizeof(*sa)) == -1) {
        if (errno == ECONNREFUSED && refused_counter < 5) {
            refused_counter++;
            errno = 0;
            sleep(5);
            continue;
        }
        ERR_PERROR_EXIT(errno != ENOENT, "Error connect");
        DPRINT("%u: retry to connect to %s\terrno = %d\n", n, SOK_NAME, errno);
        sleep(1);
        errno = 0;
    }

    DPRINT("%u: connected to %s\n", n, SOK_NAME);

    double *num_array = NULL;
    size_t num_array_len = 0;
    data_t data;
    char *path;
    memset((void *)&data, 0, sizeof(data));
    data.n = 0;
    path = (char *)queue_pop(files);
    while (path != STOP) {
        DPRINT("%u: %s\n", n, path);

        file_to_array(path, &num_array, &num_array_len, &(data.n));
        data.avg = avg(num_array, data.n);
        data.std = std(num_array, data.n);
        strcpy(data.path, path);
        write(fd_skt, &data, sizeof(data_t));
        free(path);
        path = (char *)queue_pop(files);
    }

    mtx_lock(&c->mtx);
    if (c->val < arg1->nworker) {
        (c->val)++;
        mtx_unlock(&c->mtx);
    } else {
        data.n = 0;
        data.std = -1;
        data.avg = -1;
        strcpy(data.path, STOP_STR);
        write(fd_skt, &data, sizeof(data_t));
        mtx_unlock(&c->mtx);
    }

    close(fd_skt);
    free(num_array);

    return EXIT_SUCCESS;
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
                // DPRINT("deb: %s\n", fname);
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
