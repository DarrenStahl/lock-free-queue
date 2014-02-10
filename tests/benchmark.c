#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#define BENCHMARK_OPS 0x800000

#include <stdlib.h>
#include <check.h>
#include <limits.h>
#include <time.h>
#include "../src/lock_free_queue.h"
struct timespec diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

int main() {
    struct lock_free_queue *q;
    unsigned long i, num = BENCHMARK_OPS;
    struct timespec time1, time2;

    q = create_lock_free_queue(num);
    if (q == NULL) return -1;

    clock_gettime(CLOCK_MONOTONIC_COARSE, &time1);
    for (i = 0; i < num; i++) offer_one(q, NULL);
    for (i = 0; i < num; i++) poll_one(q);
    for (i = 0; i < num; i++) offer_one(q, NULL);
    for (i = 0; i < num; i++) poll_one(q);
    clock_gettime(CLOCK_MONOTONIC_COARSE, &time2);
    printf("--------BENCHMARK--------\n");
    printf("Total time: %lu:%lu\n", diff(time1,time2).tv_sec, diff(time1,time2).tv_nsec);
    printf("Number of operations: %lu\n", (unsigned long)num * 4);
    printf("Ops/sec: %lu\n", ((unsigned long)num * 4 * 1000000000) / (diff(time1, time2).tv_nsec + diff(time1, time2).tv_sec * 1000000000));

    free_lock_free_queue(q);
}
