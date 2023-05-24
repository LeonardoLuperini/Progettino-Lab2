#include "files.h"

void concat_path(char *path, char *name, char *newpath) {
    strcpy(newpath, path);
    strcat(newpath, "/");
    strcat(newpath, name);
}

bool file_to_array(char *path, double **numbers, size_t *actual_len,
                   size_t *index) {
    FILE *f;
    char *linebuf = NULL;
    char *test = NULL;
    size_t linelen = 0;
    ssize_t resgetline;

    if ((f = fopen(path, "r")) == NULL) {
        return false;
    }

    if (*actual_len == 0) {
        (*numbers) =
            reallocarray(*numbers, (*actual_len = 100), sizeof(double));
        ERR_RET((*numbers) == NULL, false);
    }

    for (*index = 0; !feof(f); errno = 0) {
        resgetline = getline(&linebuf, &linelen, f);
        if (resgetline == -1 && errno == 0)
            break;
        else if (resgetline == -1 && errno != 0) {
            if (linebuf != NULL)
                free(linebuf);
            return false;
        }

        if (*index == *actual_len) {
            (*numbers) =
                reallocarray(*numbers, (*actual_len *= 2), sizeof(double));
            ERR_RET((*numbers) == NULL, false);
        }

        (*numbers)[(*index)] = strtod(linebuf, &test);
        if ((*numbers)[(*index)] != 0 && linebuf != test)
            (*index)++;
    }

    if (linebuf != NULL)
        free(linebuf);
    fclose(f);
    return true;
}
