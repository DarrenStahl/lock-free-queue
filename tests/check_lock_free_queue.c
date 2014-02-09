#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#define BENCHMARK_OPS 0x80000

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

START_TEST (test_next_power_of_two_simple) {
    ck_assert_int_eq(next_power_of_two(3), 4);
    ck_assert_int_eq(next_power_of_two(5), 8);
    ck_assert_int_eq(next_power_of_two(14), 16);
    ck_assert_int_eq(next_power_of_two(27), 32);
    ck_assert_int_eq(next_power_of_two(33), 64);
    ck_assert_int_eq(next_power_of_two(0x80000000 - 20), 0x80000000);
    ck_assert_int_eq(next_power_of_two(LONG_MAX), (unsigned long)LONG_MAX + 1);
}
END_TEST

START_TEST (test_next_power_of_two_no_change) {
    ck_assert_int_eq(next_power_of_two(2), 2);
    ck_assert_int_eq(next_power_of_two(4), 4);
    ck_assert_int_eq(next_power_of_two(8), 8);
    ck_assert_int_eq(next_power_of_two(16), 16);
    ck_assert_int_eq(next_power_of_two(32), 32);
    ck_assert_int_eq(next_power_of_two(0x80000000), 0x80000000);
    ck_assert_int_eq(next_power_of_two((unsigned long)LONG_MAX + 1), (unsigned long)LONG_MAX + 1);
}
END_TEST

START_TEST (test_next_power_of_two_zero) {
    ck_assert_int_eq(next_power_of_two(0), 0);
}
END_TEST

START_TEST (test_next_power_of_two_invalid) {
    ck_assert_int_eq(next_power_of_two(ULONG_MAX), 0);
    ck_assert_int_eq(next_power_of_two((ULONG_MAX >> 1) +  2), 0);
}
END_TEST

START_TEST (test_create_queue_simple) {
    struct lock_free_queue q;
    ck_assert_int_eq(create_lock_free_queue(&q, 0x80 - 1), 0x80);

    ck_assert_int_eq(q.cLength, 0x80);
    ck_assert_int_eq(q.pLength, 0x80);

    ck_assert_int_eq(q.cMask, 0x7F);
    ck_assert_int_eq(q.pMask, 0x7F);

    ck_assert(q.cQueue != NULL);
    ck_assert(q.pQueue == q.cQueue);
    
    ck_assert_int_eq(q.head, 0);
    ck_assert_int_eq(q.cachedHead, 0);
    ck_assert_int_eq(q.tail, 0);
    ck_assert_int_eq(q.cachedTail, 0);
    
    free_lock_free_queue(&q);
}
END_TEST

START_TEST (test_offer_simple) {
    struct lock_free_queue q;
    create_lock_free_queue(&q, 2);

    int x, y;
    ck_assert(offer_one(&q, (void*)&x) != -1);
    ck_assert(&x == (int*)q.cQueue[0]);

    ck_assert(offer_one(&q, (void*)&y) != -1);
    ck_assert(&y == (int*)q.cQueue[1]);
    
    free_lock_free_queue(&q);
}
END_TEST

START_TEST (test_offer_full) {
    struct lock_free_queue q;
    create_lock_free_queue(&q, 2);

    int x, y;
    ck_assert(offer_one(&q, (void*)&x) != -1);
    ck_assert((void*)&x == (int*)q.cQueue[0]);

    ck_assert(offer_one(&q, (void*)&x)!= -1);
    ck_assert((void*)&x == (int*)q.cQueue[1]);

    ck_assert(offer_one(&q, (void*)&y) == -1);
    
    free_lock_free_queue(&q);
}
END_TEST

START_TEST (test_poll_simple) {
    struct lock_free_queue q;
    create_lock_free_queue(&q, 2);

    int x, y;
    offer_one(&q, (void*)&x);

    ck_assert(poll_one(&q) == (void*)&x);
    
    free_lock_free_queue(&q);
}
END_TEST

START_TEST (test_poll_empty) {
    struct lock_free_queue q;
    create_lock_free_queue(&q, 2);

    ck_assert(poll_one(&q) == NULL);
    
    free_lock_free_queue(&q);
}
END_TEST

START_TEST (test_end_to_end) {
    struct lock_free_queue q;
    int x, i;

    create_lock_free_queue(&q, 8);

    for (i = 0; i < 8; i++) ck_assert(offer_one(&q, (void*)&x));
    for (i = 0; i < 8; i++) ck_assert(poll_one(&q) == (void*)&x);
    for (i = 0; i < 8; i++) ck_assert(offer_one(&q, (void*)&x));
    for (i = 0; i < 8; i++) ck_assert(poll_one(&q) == (void*)&x);
    free_lock_free_queue(&q);
}
END_TEST

START_TEST (benchmark_one_to_one_) {
    struct lock_free_queue q;
    int x, i, num = BENCHMARK_OPS;
    struct timespec time1, time2;

    create_lock_free_queue(&q, num);

    clock_gettime(CLOCK_REALTIME, &time1);
    for (i = 0; i < num; i++) offer_one(&q, (void*)&x);
    for (i = 0; i < num; i++) poll_one(&q);
    clock_gettime(CLOCK_REALTIME, &time2);
    printf("--------BENCHMARK--------\n");
    printf("Total time: %lu:%lu\n", diff(time1,time2).tv_sec, diff(time1,time2).tv_nsec);
    printf("Number of operations: %lu\n", (unsigned long)num * 2);
    printf("Ops/sec: %lu\n", ((unsigned long)num * 2) / ((diff(time1, time2).tv_nsec + diff(time1, time2).tv_sec * 1000000000) * 1000000000));
    free_lock_free_queue(&q);
}
END_TEST

    Suite *
lock_free_suite (void)
{
    Suite *s = suite_create ("Lock Free");

    /* Core test case */
    TCase *tc_core = tcase_create ("Core internals");
    TCase *tc_create = tcase_create ("Create queue");
    TCase *tc_offer_one = tcase_create ("Offer queue");
    TCase *tc_poll_one = tcase_create ("Poll queue");
    TCase *tc_end_to_end_one = tcase_create ("1P1C End to End");

    //tcase_set_timeout(tc_end_to_end_one, 1000000000);

    tcase_add_test (tc_core, test_next_power_of_two_simple);
    tcase_add_test (tc_core, test_next_power_of_two_no_change);
    tcase_add_test (tc_core, test_next_power_of_two_zero);
    tcase_add_test (tc_core, test_next_power_of_two_invalid);
    tcase_add_test (tc_create, test_create_queue_simple);
    tcase_add_test (tc_offer_one, test_offer_simple);
    tcase_add_test (tc_offer_one, test_offer_full);
    tcase_add_test (tc_poll_one, test_poll_simple);
    tcase_add_test (tc_poll_one, test_poll_empty);
    tcase_add_test (tc_end_to_end_one, test_end_to_end);
    tcase_add_test (tc_end_to_end_one, benchmark_one_to_one_);

    suite_add_tcase (s, tc_core);
    suite_add_tcase (s, tc_create);
    suite_add_tcase (s, tc_offer_one);
    suite_add_tcase (s, tc_poll_one);
    suite_add_tcase (s, tc_end_to_end_one);

    return s;
}

    int
main (void)
{
    int number_failed;
    Suite *s = lock_free_suite ();
    SRunner *sr = srunner_create (s);
    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

