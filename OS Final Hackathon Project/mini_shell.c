#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define MAX_JOBS 100
#define MAX_LINE 256
#define MAX_ARGS 32
#define HISTORY_SIZE 100

enum State {
    EMPTY, RUNNING, STOPPED, FINISHED
};

struct Job {
    int id;
    pid_t pid;
    char command[MAX_LINE];
    time_t start, end;
    enum State state;
    int used;
};

struct Job jobs[MAX_JOBS];
char history[HISTORY_SIZE][MAX_LINE];
int history_count = 0;
int next_id = 1;

/* Store command history */
void add_history(char *cmd) {
    if (history_count == HISTORY_SIZE) {
        memmove(history, history + 1,
                sizeof(history[0]) * (HISTORY_SIZE - 1));
        history_count--;
    }

    snprintf(history[history_count++], MAX_LINE, "%s", cmd);
}

/* Update background job states */
void update_jobs(void) {
    int status;
    pid_t result;

    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].used ||
            jobs[i].state == FINISHED)
            continue;

        do {
            result = waitpid(jobs[i].pid, &status,
                             WNOHANG | WUNTRACED | WCONTINUED);
        } while (result == -1 && errno == EINTR);

        if (result == jobs[i].pid) {
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                jobs[i].state = FINISHED;
                jobs[i].end = time(NULL);
            } else if (WIFSTOPPED(status)) {
                jobs[i].state = STOPPED;
            } else if (WIFCONTINUED(status)) {
                jobs[i].state = RUNNING;
            }
        } else if (result == -1 && errno == ECHILD) {
            jobs[i].state = FINISHED;
            jobs[i].end = time(NULL);
        }
    }
}

/* Find a job by job number */
int find_job(int id) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].used && jobs[i].id == id &&
            jobs[i].state != FINISHED)
            return i;
    }
    return -1;
}

/* Display jobs and running duration */
void list_jobs(void) {
    update_jobs();

    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].used)
            continue;

        time_t finish = (jobs[i].state == FINISHED)
                        ? jobs[i].end : time(NULL);

        double duration = difftime(finish, jobs[i].start);

        const char *state =
            jobs[i].state == RUNNING ? "Running" :
            jobs[i].state == STOPPED ? "Stopped" : "Finished";

        printf("[%d] %-8s PID:%d  Time:%.0f sec  %s\n",
               jobs[i].id, state, jobs[i].pid,
               duration, jobs[i].command);
    }
}

/* Find an available job slot */
int free_slot(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].used || jobs[i].state == FINISHED)
            return i;
    }
    return -1;
}

/* Clean up background processes on exit */
void cleanup(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].used || jobs[i].state == FINISHED)
            continue;

        if (jobs[i].state == STOPPED)
            kill(jobs[i].pid, SIGCONT);

        kill(jobs[i].pid, SIGTERM);
    }

    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].used && jobs[i].state != FINISHED) {
            while (waitpid(jobs[i].pid, NULL, 0) == -1
                   && errno == EINTR) {
                /* Keep waiting if interrupted */
            }
        }
    }
}

int main(void) {
    char input[MAX_LINE], original[MAX_LINE];
    char *args[MAX_ARGS];
    int background, argc, slot;
    pid_t pid;

    while (1) {
        update_jobs();

        printf("mini-shell$ ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        int len = strlen(input);

        while (len > 0 &&
               (input[len - 1] == ' ' ||
                input[len - 1] == '\t'))
            input[--len] = '\0';

        if (len == 0)
            continue;

        snprintf(original, sizeof(original), "%s", input);
        add_history(original);

        if (strcmp(input, "exit") == 0)
            break;

        if (strcmp(input, "jobs") == 0) {
            list_jobs();
            continue;
        }

        if (strcmp(input, "history") == 0) {
            for (int i = 0; i < history_count; i++)
                printf("%d  %s\n", i + 1, history[i]);
            continue;
        }

        if (strncmp(input, "cd", 2) == 0 &&
            (input[2] == '\0' || input[2] == ' ' ||
             input[2] == '\t')) {
            char *path = input + 2;

            while (*path == ' ' || *path == '\t')
                path++;

            if (*path == '\0')
                path = getenv("HOME");

            if (path == NULL || chdir(path) == -1)
                perror("cd");

            continue;
        }

        /* Remove & to identify a background command */
        background = 0;
        len = strlen(input);

        if (len > 0 && input[len - 1] == '&') {
            background = 1;
            input[--len] = '\0';

            while (len > 0 &&
                   (input[len - 1] == ' ' ||
                    input[len - 1] == '\t'))
                input[--len] = '\0';
        }

        argc = 0;
        char *token = strtok(input, " \t");

        while (token != NULL && argc < MAX_ARGS - 1) {
            args[argc++] = token;
            token = strtok(NULL, " \t");
        }
        args[argc] = NULL;

        if (argc == 0)
            continue;

        int job_id = 0;

        if (background) {
            slot = free_slot();

            if (slot == -1) {
                printf("Job table full!\n");
                continue;
            }

            job_id = next_id++;
        }

        /* Built-in job control commands */
        if (strcmp(args[0], "kill") == 0 ||
            strcmp(args[0], "stop") == 0 ||
            strcmp(args[0], "resume") == 0) {

            if (args[1] == NULL) {
                printf("Usage: %s job_number\n", args[0]);
                continue;
            }

            char *endptr;
            long number = strtol(args[1], &endptr, 10);

            if (*args[1] == '\0' || *endptr != '\0' ||
                number <= 0 || number > 2147483647L) {
                printf("Invalid job number\n");
                continue;
            }

            int index = find_job((int)number);

            if (index == -1) {
                printf("Job not found or already finished\n");
                continue;
            }

            int sig = strcmp(args[0], "kill") == 0
                      ? SIGTERM :
                      strcmp(args[0], "stop") == 0
                      ? SIGSTOP : SIGCONT;

            if (kill(jobs[index].pid, sig) == -1) {
                perror("Signal failed");
            } else {
                if (sig == SIGSTOP)
                    jobs[index].state = STOPPED;
                else if (sig == SIGCONT)
                    jobs[index].state = RUNNING;

                printf("Signal sent to job %ld\n", number);
            }

            continue;
        }

        pid = fork();

        if (pid == 0) {
            execvp(args[0], args);
            perror("Command failed");
            _exit(127);
        } else if (pid < 0) {
            perror("fork failed");
        } else if (background) {
            memset(&jobs[slot], 0, sizeof(jobs[slot]));

            jobs[slot].id = job_id;
            jobs[slot].pid = pid;
            jobs[slot].start = time(NULL);
            jobs[slot].state = RUNNING;
            jobs[slot].used = 1;

            snprintf(jobs[slot].command, MAX_LINE, "%s", original);

            printf("[%d] Started PID: %d\n", job_id, pid);
        } else {
            int status;

            while (waitpid(pid, &status, 0) == -1) {
                if (errno != EINTR) {
                    perror("waitpid");
                    break;
                }
            }
        }
    }

    cleanup();
    printf("Shell exited.\n");
    return 0;
}
