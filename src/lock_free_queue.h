#ifndef LOCK_FREE_QUEUE_H
#define LOCK_FREE_QUEUE_H

#include <stdio.h>

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

struct lock_free_queue{
    void** pQueue __attribute__((aligned(CACHE_LINE_SIZE)));
    unsigned long head;
    unsigned long cachedTail;
    unsigned long pMask;
    unsigned long pLength;

    void** cQueue __attribute__((aligned(CACHE_LINE_SIZE)));
    unsigned long tail;
    unsigned long cachedHead;
    unsigned long cMask;
    unsigned long cLength;
} ;

unsigned long next_power_of_two(unsigned long value);
struct lock_free_queue* create_lock_free_queue(unsigned long length);
int offer_one(struct lock_free_queue* queue, void* item);
void* poll_one(struct lock_free_queue* queue);

#endif /* LOCK_FREE_QUEUE_H */
