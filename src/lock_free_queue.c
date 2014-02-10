#include <stdlib.h>
#include "lock_free_queue.h"
#include <limits.h>

unsigned long next_power_of_two(unsigned long value) {
    unsigned long oldValue;
    if (value > ((ULONG_MAX >> 1) + 1)) return 0;

    while (((value - 1) & value) != 0) {
        oldValue = value;
        value = (value + 1);
        if (((value - 1) & value) == 0) return value;
        value |= oldValue;
    }
    return value;
}

struct lock_free_queue* create_lock_free_queue(unsigned long capacity) {
    struct lock_free_queue *queue;
    queue = malloc(sizeof(struct lock_free_queue));
    
    if (queue == NULL) return NULL;

    queue->pLength = next_power_of_two(capacity);
    queue->cLength = queue->pLength;

    if (queue->cLength == 0) {
        free(queue);
        return NULL;
    }

    queue->pMask = queue->pLength - 1;
    queue->cMask = queue->cLength - 1;

    queue->head = 0;
    queue->cachedHead = 0;
    queue->tail = 0;
    queue->cachedTail = 0;

    queue->pQueue = calloc(queue->pLength, sizeof(void*));
    queue->cQueue = queue->pQueue;

    if (queue->pQueue == NULL) {
        free(queue);
        return NULL;
    }

    return queue;
}

void free_lock_free_queue(struct lock_free_queue* queue) {
    free(queue->pQueue);
    queue->cQueue = NULL;
    free(queue);
}

int offer_one(struct lock_free_queue* queue, void* item) {
    unsigned long head = queue->head;
    long wrapPoint = head - queue->pLength;

    if (__builtin_expect ((long)queue->cachedTail <= wrapPoint, 0)) {
        queue->cachedTail = queue->tail;
        if (__builtin_expect (queue->cachedTail <= wrapPoint, 0)) {
            return -1;
        }
    }

    queue->pQueue[head & queue->pMask] = item;

    asm volatile("" ::: "memory");

    //__sync_add_and_fetch (&(queue->head), 1);
    queue->head++;
    return 1;
}

void* poll_one(struct lock_free_queue* queue) {
    void *item;
    unsigned long index;
    unsigned long tail = queue->tail;
    
    if (__builtin_expect (tail >= queue->cachedHead, 0)) {
        queue->cachedHead = queue->head;
        if (__builtin_expect (tail >= queue->cachedHead, 0)) {
            return NULL;
        }
    }
    index = tail & queue->cMask;
    item = queue->cQueue[index];

    asm volatile("" ::: "memory");

    //__sync_add_and_fetch (&(queue->tail), 1);
    queue->tail++;
    return item;
}
