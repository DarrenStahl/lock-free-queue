#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

//#define BENCHMARK_OPS 0x800000
#define BENCHMARK_OPS 0x80

#define START_PERF() { struct timespec startTime, endTime; clock_gettime(CLOCK_MONOTONIC_COARSE, &startTime);

#define END_PERF(num_ops) clock_gettime(CLOCK_MONOTONIC_COARSE, &endTime); printf("Total time: %lu:%lu\n", diff(startTime,endTime).tv_sec, diff(startTime,endTime).tv_nsec); printf("Number of operations: %lu\n", (unsigned long)(num_ops)); printf("Ops/sec: %lu\n", ((unsigned long)(num_ops) * 1000000000) / (diff(startTime, endTime).tv_nsec + diff(startTime, endTime).tv_sec * 1000000000)); }

#include <stdlib.h>
#include <check.h>
#include <limits.h>
#include <time.h>
#include <signal.h>
#include "../src/lock_free_queue.h"

static volatile int startTest;

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

void simpleBenchmark(unsigned long num);
void *producerThread_one(void *arg);
void *producerThread_mul(void *arg);
void *consumerThread_one(void *arg);
void threadedBenchmark();

struct threadArgs {
    int numProducers;
    int numConsumers;
    struct lock_free_queue *queue;
};

int main() {

    simpleBenchmark(BENCHMARK_OPS);

    threadedBenchmark(1, 1);
    //threadedBenchmark(2, 1);
    //threadedBenchmark(3, 1);

    return 1;
}

void simpleBenchmark(unsigned long num) {
    struct lock_free_queue *q;
    unsigned long n, i;
    q = create_lock_free_queue(num);
    if (q == NULL) return;
    printf("--------SIMPLE_BENCHMARK--------\n"); 
    START_PERF();
    for (n = 0; n < 100; n++) {
        for (i = 0; i < num; i++) offer_one(q, (void*)1);
        for (i = 0; i < num; i++) poll_one(q);
    }
    END_PERF(2 * 100 * num);
    free_lock_free_queue(q);
}

void threadedBenchmark(int numProducers, int numConsumers) {
    struct lock_free_queue *q;
    pthread_t *producers, *consumers;
    struct threadArgs args;
    void *status;
    int n;
    startTest = 0;
    q = create_lock_free_queue(BENCHMARK_OPS);
    if (q == NULL) return;

    producers = (pthread_t*)calloc(numProducers, sizeof(pthread_t));
    consumers = (pthread_t*)calloc(numConsumers, sizeof(pthread_t));

    args.queue = q;
    args.numProducers = numProducers;
    args.numConsumers = numConsumers;

    for(n = 0; n < numProducers; n++)  {
        if (numProducers == 0)
            pthread_create(&(producers[n]), NULL, producerThread_one, (void*)&args);
        else
            pthread_create(&(producers[n]), NULL, producerThread_mul, (void*)&args);
    }
    for(n = 0; n < numConsumers; n++) {
        if (numConsumers == 1)
            pthread_create(&(consumers[n]), NULL, consumerThread_one, (void*)&args);
    }
    sleep(1);

    printf("---THREADED_BENCHMARK_%dP_%dC---\n", numProducers, numConsumers); 
    START_PERF();
    startTest = 1;
    for(n = 0; n < numProducers; n++)
        pthread_join(producers[n], &status);
    for(n = 0; n < numConsumers; n++)
        pthread_join(consumers[n], &status);
    END_PERF(2 * 100 * BENCHMARK_OPS);

    free_lock_free_queue(q);
    free(producers);
    free(consumers);
}

void *producerThread_one(void *arg) {
    struct lock_free_queue *q;
    unsigned long n, i;
    int loopSize = 100 * ((struct threadArgs*)arg)->numConsumers;
    q = (struct lock_free_queue*)((struct threadArgs*)arg)->queue;
    unsigned long num = q->pLength;
    while(!startTest);
    for (n = 0; n < loopSize; n++) {
        for (i = 0; i < num; i++) while(offer_one(q,(void*)1) == -1);
    }
}

void *producerThread_mul(void *arg) {
    struct lock_free_queue *q;
    unsigned long n, i;
    int loopSize = 100 * ((struct threadArgs*)arg)->numConsumers;
    q = (struct lock_free_queue*)((struct threadArgs*)arg)->queue;
    unsigned long num = q->pLength;
    printf("p: %d\n", loopSize);
    while(!startTest);
    for (n = 0; n < loopSize; n++) {
        for (i = 0; i < num; i++) while(offer_mul(q,(void*)1) == -1) {
            printf("attempt p: %lu\n", i);
        };
        printf("current p: %lu\n", n);
    }
}

void *consumerThread_one(void *arg) {
    struct lock_free_queue *q;
    unsigned long n, i;
    int loopSize = 100 * ((struct threadArgs*)arg)->numProducers;
    q = (struct lock_free_queue*)((struct threadArgs*)arg)->queue;
    unsigned long num = q->cLength;
    printf("c: %d\n", loopSize);
    while(!startTest);
    for (n = 0; n < loopSize; n++) {
        for (i = 0; i < num; i++) while(poll_one(q) == NULL);
        printf("current c: %lu\n", n);
    }
}
