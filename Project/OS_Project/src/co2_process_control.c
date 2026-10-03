#include "co2_process_control.h"

/*
 * CO-2: PROCESS CONTROL
 *
 * System calls: fork(), getpid(), getppid(), wait(), _exit()
 */

/* Global flag to control zombie demo */
static int zombie_demo_enabled = 0;

void co2_demo(void)
{
    print_header("CO-2: PROCESS CONTROL");

    printf("\n  Parent PID : %d\n", getpid());
    printf("  Parent PPID: %d\n", getppid());

    /* fork() */
    printf("\n  fork()\n");
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        /* Child */
        printf("  [Child] PID=%d  PPID=%d\n", getpid(), getppid());
        sleep(1);
        printf("  [Child] _exit(0)\n");
        fflush(stdout);
        _exit(0);
    } else {
        /* Parent */
        printf("  [Parent] fork() -> child PID %d\n", pid);

        /* wait() */
        printf("\n  wait()\n");
        int status;
        pid_t child = wait(&status);

        if (child == -1) {
            perror("wait");
        } else {
            printf("  wait() -> child %d terminated\n", child);
            if (WIFEXITED(status)) {
                printf("  Exit status: %d\n", WEXITSTATUS(status));
            }
            if (WIFSIGNALED(status)) {
                printf("  Killed by signal: %d\n", WTERMSIG(status));
            }
        }
    }

    /* Zombie demo (disabled by default) */
    if (zombie_demo_enabled) {
        printf("\n  Zombie demo:\n");
        pid_t zpid = fork();
        if (zpid == 0) {
            printf("  [Child %d] Exiting...\n", getpid());
            _exit(0);
        } else {
            printf("  [Parent] Child %d exited, not waiting...\n", zpid);
            sleep(3);
            printf("  [Parent] wait() -> reaping zombie\n");
            wait(NULL);
            printf("  [Parent] Zombie reaped.\n");
        }
    }
}
