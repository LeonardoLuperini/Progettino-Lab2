#ifndef FILES_UTIL
#define FILES_UTIL

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include "error_handling_utils.h"

#define IS_DIR(dirent) (dirent->d_type == DT_DIR)
#define IS_FILE(dirent) (dirent->d_type == DT_REG)

#define NOT_THIS_DIR(name) (strcmp(name, ".") != 0)
#define NOT_PREV_DIR(name) (strcmp(name, "..") != 0)

#define IS_SUB_DIR(dirent) (IS_DIR(dirent) && 					\
							NOT_THIS_DIR(dirent->d_name) && 	\
							NOT_PREV_DIR(dirent->d_name))

/* Concatenate path and name like path + / + name
 * and return the result in newpath.
 *
 * TODO: MANCANO I CONTROLLI 
 */
void concat_path(char* path, char* name, char* newpath);

/* Get a dat file with blank spaces and double and then 
 * return an array containing all the double in the files
 *
 * Params:
 * path to the dat file
 * number is poiter to the array of double if (*numbers) == NULL
 * the function dynamicaly allocate space, and if there isn't 
 * enough space double the size
 * actual_len is len of the array
 * index is the number of doubles in the array
 *
 * Returns:
 * true if there was no error
 * false otherwise and (*numbers) get freed and set to NULL,
 * actual_len and index aren't valid inormation anymore.
 */
bool file_to_array(char* path, double** numbers, size_t* actual_len, size_t* index);

#endif
