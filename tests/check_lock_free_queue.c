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
    ck_assert_int_eq(next_power_of_two(0), 0);
}
END_TEST

START_TEST (test_next_power_of_two_invalid) {
    ck_assert_int_eq(next_power_of_two(LONG_MAX), 0);
    ck_assert_int_eq(next_power_of_two((LONG_MAX >> 1) +  2), 0);
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
}
END_TEST

START_TEST (test_offer_simple) {
    struct lock_free_queue q;
    create_lock_free_queue(&q, 2);

    int x, y;
    ck_assert(offer(&q, (void*)&x) == 0);
    ck_assert(&x == (int*)q.cQueue[0]);

    ck_assert(offer(&q, (void*)&x) == 0);
    ck_assert(&x == (int*)q.cQueue[1]);
}
END_TEST

START_TEST (test_offer_full) {
    struct lock_free_queue q;
    create_lock_free_queue(&q, 2);

    int x, y;
    ck_assert(offer(&q, (void*)&x) == 0);
    ck_assert((void*)&x == (int*)q.cQueue[0]);

    ck_assert(offer(&q, (void*)&x) == 0);
    ck_assert((void*)&x == (int*)q.cQueue[1]);

    ck_assert(offer(&q, (void*)&y) == -1);
}
END_TEST

START_TEST (test_poll_simple) {
    struct lock_free_queue q;
    create_lock_free_queue(&q, 2);

    int x, y;
    offer(&q, (void*)&x);

    ck_assert(poll(&q) == (void*)&x);
}
END_TEST

START_TEST (test_poll_empty) {
    struct lock_free_queue q;
    create_lock_free_queue(&q, 2);

    ck_assert(poll(&q) == NULL);
}
END_TEST

    Suite *
lock_free_suite (void)
{
    Suite *s = suite_create ("Lock Free");

    /* Core test case */
    TCase *tc_core = tcase_create ("Core internals");
    TCase *tc_create = tcase_create ("Create queue");
    TCase *tc_offer = tcase_create ("Offer queue");
    TCase *tc_poll = tcase_create ("Poll queue");

    tcase_add_test (tc_core, test_next_power_of_two_simple);
    tcase_add_test (tc_core, test_next_power_of_two_no_change);
    tcase_add_test (tc_core, test_next_power_of_two_zero);
    tcase_add_test (tc_core, test_next_power_of_two_invalid);
    tcase_add_test (tc_create, test_create_queue_simple);
    tcase_add_test (tc_offer, test_offer_simple);
    tcase_add_test (tc_offer, test_offer_full);
    tcase_add_test (tc_poll, test_poll_simple);
    tcase_add_test (tc_poll, test_poll_empty);

    suite_add_tcase (s, tc_core);
    suite_add_tcase (s, tc_create);
    suite_add_tcase (s, tc_offer);
    suite_add_tcase (s, tc_poll);

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
