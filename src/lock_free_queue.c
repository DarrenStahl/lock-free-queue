#include <stdlib.h>
#include "lock_free_queue.h"
#include <limits.h>

long next_power_of_two(long value) {
    long oldValue;
    if (value > ((LONG_MAX >> 1) + 1)) return 0;

    while (((value - 1) & value) != 0) {
        oldValue = value;
        value = (value + 1);
        if (((value - 1) & value) == 0) return value;
        value |= oldValue;
    }
    return value;
}

long create_lock_free_queue(struct lock_free_queue *queue, 
        long capacity) {
    queue->pLength = next_power_of_two(capacity);
    queue->cLength = queue->pLength;
    if (queue->cLength == 0) return -1;

    queue->pMask = queue->pLength - 1;
    queue->cMask = queue->cLength - 1;

    queue->head = 0;
    queue->cachedHead = 0;
    queue->tail = 0;
    queue->cachedTail = 0;

    queue->pQueue = calloc(queue->pLength, sizeof(void*));
    queue->cQueue = queue->pQueue;

    return queue->pLength;
}

void free_lock_free_queue(struct lock_free_queue* queue) {
    free(queue->pQueue);
    queue->cQueue = NULL;
}

int offer(struct lock_free_queue* queue, void* item) {
    return 0;
}

void* poll(struct lock_free_queue* queue) {
    return NULL;
}
