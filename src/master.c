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
} arg_t;

static void get_cli_input(int argc, char **argv, size_t *nworker, char **path);
static bool str_is_size(char *str);
static void search_datfiles(char *path, queue_t *files);
static void *worker(void *arg);

int main(int argc, char *argv[]) {
    size_t nworker;
    char *path;
    arg_t arg;

    get_cli_input(argc, argv, &nworker, &path);
    DPRINT("nworker: %lu\npath: %s\n", nworker, path);
    pthread_t tids[nworker];

    queue_t *files = queue_init();
    ERR_PRINT_EXIT(files == NULL, "Error queue_init\n");

    struct sockaddr_un sa;
    strncpy(sa.sun_path, SOK_NAME, SUN_MAX_LEN);
    sa.sun_family = AF_UNIX;

    arg.files = files;
    arg.sa = &sa;
    arg.counter = counter_init(1);
    arg.nworker = nworker;
    for (size_t i = 0; i < nworker; i++)
        sthread_create(&tids[i], worker, (void *)&arg);

    search_datfiles(path, files);

    for (size_t i = 0; i < nworker; i++)
        ERR_PRINT_EXIT(!queue_push(files, STOP), "Error queue_push\n");

    for (size_t i = 0; i < nworker; i++)
        sthread_join(tids[i], NULL);

    queue_destroy(files);

    return EXIT_SUCCESS;
}

static void *worker(void *arg) {
    arg_t *arg1 = (arg_t *)arg;
    struct sockaddr_un *sa = (struct sockaddr_un *)arg1->sa;
    queue_t *files = (queue_t *)arg1->files;

    data_t data;
    char *path;
    double *num_array = NULL;
    size_t num_array_len = 0;

    int fd_skt;

    DPRINT("%lu: Hello\n", pthread_self());

    // Soket
    fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);
    ERR_PERROR_EXIT(fd_skt == -1, "Error socket");
    DPRINT("%lu: connecting to %s\n", pthread_self(), SOK_NAME);
    while (connect(fd_skt, (struct sockaddr *)sa, sizeof(*sa)) == -1) {
        ERR_PERROR_EXIT(errno != ENOENT, "Error connect");
        DPRINT("%lu: retry to connect to %s\terrno = %d\n", pthread_self(),
               SOK_NAME, errno);
        sleep(1);
        errno = 0;
    }

    data.n = 0;
    path = (char *)queue_pop(files);
    while (path != STOP) {
        DPRINT("%lu: %s\n", pthread_self(), path);

        file_to_array(path, &num_array, &num_array_len, &(data.n));
        data.avg = avg(num_array, data.n);
        data.std = std(num_array, data.n);
        strcpy(data.path, path);
        DPRINT("%lu: %lu\t%lf\t%lf\t%s\n", pthread_self(), data.n, data.avg,
               data.std, data.path);
        write(fd_skt, &data, sizeof(data_t));
        free(path);
        path = (char *)queue_pop(files);
    }

    mtx_lock(&arg1->counter->mtx);
    if (arg1->counter->val < arg1->nworker) {
        (arg1->counter->val)++;
        DPRINT("counter: %d\n", arg1->counter->val);
        mtx_unlock(&arg1->counter->mtx);
    } else {
        mtx_unlock(&arg1->counter->mtx);
        data.n = 0;
        data.std = -1;
        data.avg = -1;
        strcpy(data.path, STOP_STR);
        write(fd_skt, &data, sizeof(data_t));
    }

    close(fd_skt);
    unlink(SOK_NAME);

    return EXIT_SUCCESS;
}

static void search_datfiles(char *path, queue_t *files) {
    DIR *dir;
    char *newpath;
    struct dirent *direlem;
    char *fname;
    size_t len_fname;
    char *fext;

    dir = opendir(path);
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
            fext = fname + len_fname - 4; // TODO: use strrchr
            if (strcmp(fext, ".dat") == 0) {
                newpath = malloc(sizeof(char) * PATH_MAX);
                ERR_PRINT_EXIT(newpath == NULL, "Error malloc\n");
                DPRINT("deb: %s\n", fname);
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
