#include <stdlib.h>
#include "lock_free_queue.h"
#include <limits.h>

/*! \brief Calculates the next power of two.
 *  
 *  The next power of two is used in order to create a queue size of a
 *  two. This is used in order to allows the mod bithack to avoid a 
 *  division.
 *
 *  Returns:
 *      0 if the next power of two is not a valid long integer value
 *      else the closest power of two that is larger than value
 */
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

/*! \brief Creats a lock free queue of atleast capacity length.
 *
 *  Allocates and fills in all the necessary data for a lock free
 *  queue. free_lock_free_queue() must be called to free the alocated memory.
 *  See the struct declatation in lock_free_queue.h for reasoning on
 *  the duplication of variables.
 *
 *  Returns:
 *      A pointer to an initialized lock_free_queue on succes.
 *      NULL on failure to initialize the queue.
 */
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

    //Pre-calculate the mask to be all ones before the power.
    queue->pMask = queue->pLength - 1;
    queue->cMask = queue->cLength - 1;

    queue->head = 0;
    queue->cachedHead = 0;
    queue->tail = 0;
    queue->cachedTail = 0;

    //Allocate the queue
    queue->pQueue = calloc(queue->pLength, sizeof(void*));
    queue->cQueue = queue->pQueue;

    if (queue->pQueue == NULL) {
        free(queue);
        return NULL;
    }

    return queue;
}

/*! \brief Free a lock free queue.
 *
 *  Frees a lock_free_queue allocated by create_lock_free_queue().
 *  Does not free any user data pointers left in the queue.
 *
 *  Returns:
 *      nothing
 */
void free_lock_free_queue(struct lock_free_queue* queue) {
    free(queue->pQueue);
    queue->cQueue = NULL;
    free(queue);
}

/*! \brief Offer an item to the lock_free_queue. Single producer only.
 *
 *  Offers an item to a valid lock free queue. This method will not block
 *  on a full queue, but rather just return a -1. It is up to the user to
 *  block if they so please. This function will cause unexpected behaviour 
 *  if multiple producers are used on the same queue.
 *
 *  Returns:
 *      -1 on failure to insert item into the queue.
 *      1 on success.
 */
int offer_one(struct lock_free_queue* queue, void* item) {
    unsigned long head = queue->head;
    long wrapPoint = head - queue->pLength;

    //Try our cached tail, if this fails we still have a chance.
    //__builtin_expect() is to force branch prediction to not pipeline
    //inside the if block.
    if (__builtin_expect ((long)queue->cachedTail <= wrapPoint, 0)) {
        //Okay, we can still be okay if the current tail is not the cached tail
        queue->cachedTail = queue->tail;
        if (__builtin_expect (queue->cachedTail <= wrapPoint, 0)) {
            return -1;
        }
    }

    queue->pQueue[head & queue->pMask] = item;

    //Memory barrier. This prevents an increment of the head before the queue is
    //safe to access. This does not force buffer flush.
    asm volatile("" ::: "memory");

    queue->head++;
    return 1;
}

/*! \brief Offer an item to the lock_free_queue. Supports multiple producers.
 *
 *  Offers an item to a valid lock free queue. This method will not block
 *  on a full queue, but rather just return a -1. It is up to the user to
 *  block if they so please. This function can be used if multiple
 *  producers are working on the same queue, but can NOT be used on the 
 *  same queue as offer_one().
 *
 *  Returns:
 *      -1 on failure to insert item into the queue.
 *      1 on success.
 */
int offer_mul(struct lock_free_queue* queue, void* item) {
    unsigned long head;
    unsigned long index, mask, length;
    void ** localQueue;
    long wrapPoint;

    mask = queue->pMask;
    length = queue->pLength;
    localQueue = queue->pQueue;

    do {
        head = queue->head;
        wrapPoint = head - length;

        if (__builtin_expect ((long)queue->cachedTail <= wrapPoint, 0)) {
            queue->cachedTail = queue->tail;
            if (__builtin_expect (queue->cachedTail <= wrapPoint, 0)) {
                return -1;
            }
        }
        index = head & mask;
    } while (!__sync_bool_compare_and_swap(&localQueue[index], NULL, item));

    //Memory barrier. This prevents an increment of the head before the queue is
    //safe to access. This does not force buffer flush.
    asm volatile("" ::: "memory");

    queue->head++;
    return 1;
}

/*! \brief Poll the queue for a new item. Single consumer only.
 *
 *  Polls the queue in an attempt to get the next item in the queue.
 *  This method will not block, but rahter return NULL if the queue is
 *  currently empty. It is up to the user implementation to block if
 *  they wish to do so. This function will cause unknown behaviour if 
 *  multiple consumers are used on the same queue.
 *
 *  Returns:
 *      The pointer as inserted, on success.
 *      NULL on failure.
 */
void* poll_one(struct lock_free_queue* queue) {
    void *item;
    void **localQueue;
    unsigned long index;
    unsigned long tail = queue->tail;

    localQueue = queue->cQueue;
    
    //Try our cached head, this is to save a cache invalidation of 
    //the produceron each poll if we are well apart and not in danger
    //of being empty.
    //Same __builtin_expect as offer_one(), forces my branch prediction
    if (__builtin_expect (tail >= queue->cachedHead, 0)) {
        queue->cachedHead = queue->head;
        //Second chance, was our cached tail correct?
        if (__builtin_expect (tail >= queue->cachedHead, 0)) {
            return NULL;
        }
    }
    index = tail & queue->cMask;
    item = localQueue[index];
    localQueue[index] = NULL;

    //Memory fence as above, prevents re-ordering of instructions.
    asm volatile("" ::: "memory");

    queue->tail++;
    return item;
}
