#ifndef LOCK_FREE_QUEUE_H
#define LOCK_FREE_QUEUE_H

#include <stdio.h>

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

struct lock_free_queue{
    int head __attribute__((aligned(CACHE_LINE_SIZE)));
    int cachedTail;
    int pItemSize;
    int pMask;
    int pLength;

    int tail __attribute__((aligned(CACHE_LINE_SIZE)));
    int cachedHead;
    int cItemSize;
    int cMask;
    int cLength;
} ;

int nextPowerOfTwo(int value) {
    if ((value - 1) & value == 0) return value;

}
#endif /* LOCK_FREE_QUEUE_H */
