#ifndef THREAD_SAFE_QUEUE
#define THREAD_SAFE_QUEUE

#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pthread_utils.h"
#include "error_handling_utils.h"

typedef struct qnode_t {
    void *data;
    struct qnode_t *next;
} qnode_t;

typedef struct {
    qnode_t *head;
	qnode_t *tail;
    pthread_mutex_t mtx;
	pthread_cond_t cond;
} queue_t;

/* Initilize a qnode.
 *
 * Params:
 * pointer to data.
 *
 * Returns: 
 * NULL if there was an error and errno is set 
 * else return a pointer to the queue.
 *
 */
qnode_t* qnode_init(void *data);

/* Initilize a queue.
 *
 * Returns: 
 * NULL if there was an error 
 * else return a pointer to the queue.
 *
 */
queue_t* queue_init(void);

/* Insert data at the end of the queue.
 * 
 * Params:
 * Pointer to a queue that you want to update.
 * Pointer to data the you want to insert.
 *
 * Returns:
 * true if succeeded else false
 *
 */
bool queue_push(queue_t *queue, void *data);

/* Extract data from the head of the queue,
 * if the queue is empty wait.
 * 
 * Params:
 * Pointer to a queue that you want to update.
 *
 * Returns:
 * data of the fist node.
 *
 */
void *queue_pop(queue_t *queue);

/* Destroy a queue
 * 
 * Params:
 * Pointer to a queue that you want to destroy.
 * That pointer now point to an empty queue.
 *
 */
void queue_destroy(queue_t *queue);

/* Destroy a queue and free each data parameter
 * 
 * Params:
 * Pointer to a queue that you want to destroy.
 * That pointer now point to an empty queue.
 *
 */
void queue_destroy_fd(queue_t *queue);

#endif
