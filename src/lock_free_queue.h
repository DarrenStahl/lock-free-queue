#ifndef LOCK_FREE_QUEUE_H
#define LOCK_FREE_QUEUE_H

#include <stdio.h>

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

struct lock_free_queue{
    void** pQueue __attribute__((aligned(CACHE_LINE_SIZE)));
    long head;
    long cachedTail;
    long pMask;
    long pLength;

    void** cQueue __attribute__((aligned(CACHE_LINE_SIZE)));
    long tail;
    long cachedHead;
    long cMask;
    long cLength;
} ;

long next_power_of_two(long value);
long create_lock_free_queue(struct lock_free_queue* queue, long length);
int offer(struct lock_free_queue* queue, void* item);
void* poll(struct lock_free_queue* queue);

#endif /* LOCK_FREE_QUEUE_H */
