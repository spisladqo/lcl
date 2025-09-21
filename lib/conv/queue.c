#include "queue.h"

#include <stdlib.h>

static lcl_queue_node_t *create_node(lcl_queue_node_t *next, void *data) {
    lcl_queue_node_t *node = malloc(sizeof(lcl_queue_node_t));
    if (!node) return NULL;
    node->next = next;
    node->data = data;
    return node;
}

static void destroy_node(lcl_queue_node_t *node) { free(node); }

int lcl_queue_push(lcl_queue_t *queue, void *data) {
    if (!queue) {
        return 1;
    }

    lcl_queue_node_t *tail = queue->tail;
    lcl_queue_node_t *new_tail = create_node(NULL, data);
    if (!new_tail) {
        return 1;
    }

    if (!queue->tail) {
        queue->head = new_tail;
        queue->tail = new_tail;
    } else {
        queue->tail->next = new_tail;
        queue->tail = new_tail;
    }

    return 0;
}

void *lcl_queue_pop(lcl_queue_t *queue) {
    if (!queue || !queue->head) {
        return NULL;
    }

    lcl_queue_node_t *head = queue->head;
    lcl_queue_node_t *new_head = head->next;
    queue->head = new_head;
    if (queue->tail == head) {
        queue->tail = new_head;
    }

    void *data = head->data;
    destroy_node(head);

    return data;
}
