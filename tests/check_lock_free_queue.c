#include <stdlib.h>
#include <check.h>
#include <limits.h>
#include "../src/lock_free_queue.h"

START_TEST (test_next_power_of_two_simple) {
    ck_assert_int_eq(next_power_of_two(3), 4);
    ck_assert_int_eq(next_power_of_two(5), 8);
    ck_assert_int_eq(next_power_of_two(14), 16);
    ck_assert_int_eq(next_power_of_two(27), 32);
    ck_assert_int_eq(next_power_of_two(33), 64);
    ck_assert_int_eq(next_power_of_two(0x80000000 - 20), 0x80000000);
}
END_TEST

START_TEST (test_next_power_of_two_no_change) {
    ck_assert_int_eq(next_power_of_two(2), 2);
    ck_assert_int_eq(next_power_of_two(4), 4);
    ck_assert_int_eq(next_power_of_two(8), 8);
    ck_assert_int_eq(next_power_of_two(16), 16);
    ck_assert_int_eq(next_power_of_two(32), 32);
    ck_assert_int_eq(next_power_of_two(0x80000000), 0x80000000);
}
END_TEST

START_TEST (test_next_power_of_two_zero) {
    ck_assert_int_eq(next_power_of_two(0), 1);
}
END_TEST

START_TEST (test_next_power_of_two_invalid) {
    ck_assert_int_eq(next_power_of_two(LONG_MAX), -1);
    ck_assert_int_eq(next_power_of_two((LONG_MAX >> 1) + 2), -1);
}
END_TEST

int main() {
    return 0;
}
