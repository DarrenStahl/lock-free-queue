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

unsigned long create_lock_free_queue(struct lock_free_queue *queue, 
        unsigned long capacity) {
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

int offer_one(struct lock_free_queue* queue, void* item) {
    unsigned long head = queue->head;
    long wrapPoint = head - queue->pLength;

    if ((long)queue->cachedTail <= wrapPoint) {
        queue->cachedTail = queue->tail;
        if (queue->cachedTail <= wrapPoint) {
            return -1;
        }
    }

    queue->pQueue[head & queue->pMask] = item;
    queue->head++;
    return 1;
}

void* poll_one(struct lock_free_queue* queue) {
    void *item;
    unsigned long index;
    unsigned long tail = queue->tail;
    
    if (tail < queue->cachedHead) {
        queue->cachedHead = queue->head;
        if (tail < queue->cachedHead) {
            return NULL;
        }
    }
    index = tail & queue->cMask;
    item = queue->cQueue[index];

    queue->tail++;
    return item;
}
