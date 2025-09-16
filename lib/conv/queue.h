#include <pthread.h>

typedef struct lcl_queue_node {
    struct lcl_queue_node* next;
    void* data;
} lcl_queue_node_t;

typedef struct lcl_queue {
    lcl_queue_node_t* head;
    lcl_queue_node_t* tail;
} lcl_queue_t;

int lcl_queue_push(lcl_queue_t* queue, pthread_mutex_t* lock, void* data);
void* lcl_queue_pop(lcl_queue queue, pthread_mutex_t* lock);
