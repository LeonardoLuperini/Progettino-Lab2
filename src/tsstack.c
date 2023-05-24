#include "tsstack.h"

snode_t *snode_init(void *data) {
    snode_t *newnode = malloc(sizeof(snode_t));
    if (newnode != NULL) {
        newnode->data = data;
        newnode->prev = NULL;
    }
    return newnode;
}

stack_t *stack_init(void) {
    stack_t *s = malloc(sizeof(stack_t));
    ERR_RET(s == NULL, NULL);
    s->head = NULL;
    mtx_init(&s->mtx);
    if (pthread_cond_init(&s->cond, NULL) != 0) {
        mtx_destroy(&s->mtx);
        free(s);
        return NULL;
    }

    return s;
}

bool stack_push(stack_t *stack, void *data) {
    snode_t *newnode = snode_init(data);
    if (newnode == NULL)
        return false;
    mtx_lock(&stack->mtx);

    newnode->prev = stack->head;
    stack->head = newnode;

    cond_signal(&stack->cond);
    mtx_unlock(&stack->mtx);
    return true;
}

void *stack_pop(stack_t *stack) {
    void *data;
    snode_t *old_node;
    mtx_lock(&stack->mtx);
    while (stack->head == NULL)
        cond_wait(&stack->cond, &stack->mtx);

    data = stack->head->data;
    old_node = stack->head;
    stack->head = stack->head->prev;

    mtx_unlock(&stack->mtx);
    free(old_node);

    return data;
}

void stack_destroy(stack_t *stack) {
    snode_t *node_to_free;
    while (stack->head != NULL) {
        node_to_free = stack->head;
        stack->head = stack->head->prev;
        free(node_to_free);
    }
    mtx_destroy(&stack->mtx);
    cond_destroy(&stack->cond);
    free(stack);
}

void stack_destroy_fd(stack_t *stack) {
    snode_t *node_to_free;
    while (stack->head != NULL) {
        node_to_free = stack->head;
        stack->head = stack->head->prev;
        free(node_to_free->data);
        free(node_to_free);
    }
    mtx_destroy(&stack->mtx);
    cond_destroy(&stack->cond);
    free(stack);
}
