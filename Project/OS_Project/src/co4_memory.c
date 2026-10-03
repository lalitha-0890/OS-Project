#include "co4_memory.h"
#include <stdint.h>

/*
 * CO-4: MEMORY MANAGEMENT
 *
 * System calls / APIs used: fork(), malloc(), free()
 */

/* Global variable - lives in the data segment */
static int global_value = 100;

void co4_demo(void)
{
    print_header("CO-4: MEMORY DIAGNOSTIC");

    /* Allocate heap memory */
    int stack_var = 100;
    int *heap_var = malloc(sizeof(int));
    if (heap_var == NULL) {
        perror("malloc");
        return;
    }
    *heap_var = 100;

    printf("\n  global_value (data) = %d at %p\n",
           global_value, (void *)&global_value);
    printf("  stack_var   (stack) = %d at %p\n",
           stack_var, (void *)&stack_var);
    printf("  heap_var    (heap)  = %d at %p\n",
           *heap_var, (void *)heap_var);

    /* Fork */
    printf("\n  Calling fork()...\n");
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        free(heap_var);
        return;
    }

    if (pid == 0) {
        /* Child process */
        printf("  [Child  PID=%d]\n", getpid());

        /* Modify variables in child */
        global_value = 200;
        *heap_var = 200;
        stack_var = 200;

        printf("  [Child]  global_value = %d at %p\n",
               global_value, (void *)&global_value);
        printf("  [Child]  *heap_var    = %d at %p\n",
               *heap_var, (void *)heap_var);
        printf("  [Child]  stack_var   = %d at %p\n",
               stack_var, (void *)&stack_var);

        free(heap_var);
        printf("  [Child]  Memory freed.\n");
        fflush(stdout);
        _exit(0);
    } else {
        /* Parent process */
        printf("  [Parent PID=%d]\n", getpid());

        /* Wait for child */
        int status;
        wait(&status);

        printf("  [Parent] global_value = %d at %p\n",
               global_value, (void *)&global_value);
        printf("  [Parent] *heap_var    = %d at %p\n",
               *heap_var, (void *)heap_var);
        printf("  [Parent] stack_var   = %d at %p\n",
               stack_var, (void *)&stack_var);

        /* Verify CoW */
        if (global_value == 100 && *heap_var == 100 && stack_var == 100) {
            printf("\n  Copy-on-Write: VERIFIED\n");
        } else {
            printf("\n  Copy-on-Write: FAILED\n");
        }

        /* Memory error demonstration */
        printf("\n  Testing malloc error handling...\n");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Walloc-size-larger-than="
        void *bad_ptr = malloc(SIZE_MAX);
#pragma GCC diagnostic pop
        if (bad_ptr == NULL) {
            printf("  malloc() returned NULL, errno = %d (%s)\n",
                   errno, strerror(errno));
        } else {
            free(bad_ptr);
        }

        free(heap_var);
        printf("  [Parent] Memory freed.\n");
    }
}
