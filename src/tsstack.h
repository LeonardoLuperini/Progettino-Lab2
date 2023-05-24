#ifndef THREAD_SAFE_STACK
#define THREAD_SAFE_STACK

#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pthread_utils.h"
#include "error_handling_utils.h"

typedef struct snode_t {
    void *data;
    struct snode_t *prev;
} snode_t;

typedef struct {
    snode_t *head;
    pthread_mutex_t mtx;
	pthread_cond_t cond;
} stack_t;

/* Initilize a node.
 *
 * Params:
 * pointer to data.
 *
 * Returns: 
 * NULL if there was an error and errno is set 
 * else return a pointer to the stack.
 *
 */
snode_t* snode_init(void *data);

/* Initilize a stack.
 *
 * Returns: 
 * NULL if there was an error 
 * else return a pointer to the stack.
 *
 */
stack_t* stack_init(void);

/* Insert data in the stack.
 * 
 * Params:
 * Pointer to a stack that you want to update.
 * Pointer to data the you want to insert.
 *
 * Returns:
 * true if succeeded else false
 *
 */
bool stack_push(stack_t *stack, void *data);

/* Extract data from the stack,
 * if the stack is empty wait.
 * 
 * Params:
 * Pointer to a stack that you want to update.
 *
 * Returns:
 * data of the fist node.
 *
 */
void *stack_pop(stack_t *stack);

/* Destroy a stack
 * 
 * Params:
 * Pointer to a stack that you want to destroy.
 * That pointer now point to an empty stack.
 *
 */
void stack_destroy(stack_t *stack);

/* Destroy a stack and free each data parameter
 * 
 * Params:
 * Pointer to a stack that you want to destroy.
 * That pointer now point to an empty stack.
 *
 */
void stack_destroy_fd(stack_t *stack);


#endif
