#include <stdlib.h>
#include <check.h>
#include "../src/lock_free_queue.h"

void
setup (void)
{
}

void
teardown (void)
{
}

START_TEST(test_blank) {
    int i = 0;
    ck_assert_int_eq (i, 0);
}
