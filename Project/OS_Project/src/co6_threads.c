#include "co6_threads.h"

/*
 * CO-6: CONCURRENCY AND SYNCHRONIZATION
 *
 * APIs used: pthread_create(), pthread_join(),
 *            pthread_mutex_lock(), pthread_mutex_unlock()
 */

#define NUM_THREADS 4
#define INCREMENTS_PER_THREAD 100000

/* Shared counter (will be accessed by all threads) */
static long shared_counter = 0;

/* Mutex for Part B */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* Thread argument structure */
typedef struct {
    int thread_id;
    int use_mutex;
} thread_arg_t;

/* Worker thread function */
static void *counter_thread(void *arg)
{
    thread_arg_t *targs = (thread_arg_t *)arg;

    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        if (targs->use_mutex) {
            pthread_mutex_lock(&mutex);
            shared_counter++;
            pthread_mutex_unlock(&mutex);
        } else {
            shared_counter++;
        }
    }

    printf("  [Thread %d] Done\n", targs->thread_id);

    free(targs);
    return NULL;
}

/* ---- PART A: Race Condition ---- */
static void part_a_race_condition(void)
{
    shared_counter = 0;

    pthread_t threads[NUM_THREADS];

    printf("\n  Creating %d threads (%d increments each)...\n",
           NUM_THREADS, INCREMENTS_PER_THREAD);

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_arg_t *arg = malloc(sizeof(thread_arg_t));
        if (arg == NULL) {
            perror("malloc");
            return;
        }
        arg->thread_id = i;
        arg->use_mutex = 0;

        if (pthread_create(&threads[i], NULL, counter_thread, arg) != 0) {
            perror("pthread_create");
            free(arg);
            return;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
        }
    }

    printf("  Expected: %d\n", NUM_THREADS * INCREMENTS_PER_THREAD);
    printf("  Actual:   %ld\n", shared_counter);

    if (shared_counter != NUM_THREADS * INCREMENTS_PER_THREAD) {
        printf("  Result:   RACE CONDITION DETECTED\n");
    } else {
        printf("  Result:   No race detected this run\n");
    }
}

/* ---- PART B: Mutex Solution ---- */
static void part_b_mutex(void)
{
    shared_counter = 0;

    pthread_t threads[NUM_THREADS];

    printf("\n  Creating %d threads with mutex...\n", NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_arg_t *arg = malloc(sizeof(thread_arg_t));
        if (arg == NULL) {
            perror("malloc");
            return;
        }
        arg->thread_id = i;
        arg->use_mutex = 1;

        if (pthread_create(&threads[i], NULL, counter_thread, arg) != 0) {
            perror("pthread_create");
            free(arg);
            return;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
        }
    }

    printf("  Expected: %d\n", NUM_THREADS * INCREMENTS_PER_THREAD);
    printf("  Actual:   %ld\n", shared_counter);

    if (shared_counter == NUM_THREADS * INCREMENTS_PER_THREAD) {
        printf("  Result:   PASSED\n");
    } else {
        printf("  Result:   FAILED\n");
    }
}

/* ---- Main CO-6 entry point ---- */
void co6_demo(void)
{
    print_header("CO-6: CONCURRENCY DIAGNOSTIC");

    printf("\n  Threads: %d\n", NUM_THREADS);
    printf("  Increments per thread: %d\n", INCREMENTS_PER_THREAD);

    printf("\n  Without mutex:\n");
    part_a_race_condition();
    pause_output();

    printf("\n  With mutex:\n");
    part_b_mutex();
}
