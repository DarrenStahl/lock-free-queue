## Process this file with automake to produce Makefile.in

TESTS = check_lock_free_queue
check_PROGRAMS = check_lock_free_queue benchmark
check_lock_free_queue_SOURCES = check_lock_free_queue.c $(top_builddir)/src/lock_free_queue.h
check_lock_free_queue_CFLAGS = @CHECK_CFLAGS@ -lrt
check_lock_free_queue_LDADD = $(top_builddir)/src/liblock_free_queue.la @CHECK_LIBS@
benchmark_SOURCES = benchmark.c $(top_builddir)/src/lock_free_queue.h
benchmark_CFLAGS = @CHECK_CFLAGS@ -lrt
benchmark_LDADD = $(top_builddir)/src/liblock_free_queue.la @CHECK_LIBS@
