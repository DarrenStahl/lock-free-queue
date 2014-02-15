#ifndef LOCK_FREE_QUEUE_H
#define LOCK_FREE_QUEUE_H

#include <stdio.h>

//Assume 64 byte cache lines unless told otherwise.
//Must be a power of 2.
#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

//Lock free queue struct.
struct lock_free_queue{
    //*******************//*
    //DO NOT CHANGE ORDER//
    //*******************//*

    /* Order is important, as this struct packs each thread's
     * needed data on to its own cache line. This prevents 
     * "false sharing" of variables on the same cache line.
     *
     * Each thread has it's own copy of any static variables, 
     * as there is no need to "steal" the other thread's 
     * cache line if nothing is going to change.
     *
     * The __attribute__((aligned(byte))) forces the variable to
     * be aligned to the byte boundry. This creates two cache lines,
     * one for each thread.
     */
    void** pQueue __attribute__((aligned(CACHE_LINE_SIZE)));
    unsigned long head;
    unsigned long cachedTail;
    unsigned long pMask;
    unsigned long pLength;
    unsigned long currentProducers;
    unsigned long finishedProducers;

    void** cQueue __attribute__((aligned(CACHE_LINE_SIZE)));
    unsigned long tail;
    unsigned long cachedHead;
    unsigned long cMask;
    unsigned long cLength;
} ;

//Function declarations
unsigned long next_power_of_two(unsigned long value);
struct lock_free_queue* create_lock_free_queue(unsigned long length);
void free_lock_free_queue(struct lock_free_queue*);
int offer_one(struct lock_free_queue* queue, void* item);
void* poll_one(struct lock_free_queue* queue);

#endif /* LOCK_FREE_QUEUE_H */
