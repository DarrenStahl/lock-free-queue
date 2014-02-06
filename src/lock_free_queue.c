#include <stdlib.h>
#include "lock_free_queue.h"

long next_power_of_two(long value) {
    if (((value - 1) & value) == 0) return value;
    return -1;
}

long create_lock_free_queue(struct lock_free_queue *queue, 
        long capacity) {
    return -1;
}

int offer(struct lock_free_queue* queue, void* item) {
    return 0;
}

void* poll(struct lock_free_queue* queue) {
    return NULL;
}
