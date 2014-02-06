#include <stdlib.h>
#include "lock_free_queue.h"

long next_power_of_two(long value) {
    if (((value - 1) & value) == 0) return value;
    return -1;
}

int create_queue(struct queue_t *queue, 
        long capacity, 
        size_t item_size) {
    return -1;
}
