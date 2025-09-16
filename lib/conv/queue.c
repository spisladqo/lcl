#include <stdlib.h>
#include "queue.h"

static lcl_queue_node_t* create_node(lcl_queue_node_t* next, void* data) {
    lcl_queue_node_t* node = malloc(sizeof(*lcl_queue_node_t));
    if (!node) return NULL;
    node.next = next;
    node.data = data;
    return node;
}

static void destroy_node(lcl_queue_node_t* node) {
    free(node);
}

int lcl_queue_push(lcl_queue_t* queue, pthread_mutex_t* lock, void* data) {
    if (!queue || !lock) {
        return 1;
    }

    pthread_mutex_lock(lock);

    lcl_queue_node_t* tail = queue->tail;
    lcl_queue_node_t* new_tail = create_node(NULL, data);
    queue->tail = new_tail;
    tail->next = new_tail;

    pthread_mutex_unlock(lock);
}

void* lcl_queue_pop(lcl_queue_t* queue, pthread_mutex_t* lock) {
    if (!queue || !lock) {
        return NULL;
    }

    pthread_mutex_lock(lock);

    lcl_queue_node_t* head = queue->head;
    lcl_queue_node_t* new_head = head->next;
    queue->head = new_head;

    pthread_mutex_unlock(lock);

    void* data = head->data;
    destroy_node(head);
    
    return data;
}
