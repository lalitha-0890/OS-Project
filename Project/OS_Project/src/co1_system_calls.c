#include "co1_system_calls.h"

/*
 * CO-1: SYSTEM CALLS
 *
 * System calls: pipe(), fork(), read(), write(), open(), close(), mkfifo()
 */

void co1_demo(void)
{
    print_header("CO-1: SYSTEM CALLS");

    /* pipe() */
    int pipefd[2];
    printf("\n  pipe()\n");
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }
    printf("  read fd  = %d\n", pipefd[0]);
    printf("  write fd = %d\n", pipefd[1]);

    /* fork() */
    printf("\n  fork()\n");
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid == 0) {
        /* Child */
        close(pipefd[1]);
        char buf[BUFFER_SIZE];
        ssize_t n = read(pipefd[0], buf, BUFFER_SIZE - 1);
        if (n == -1) {
            perror("read");
            close(pipefd[0]);
            _exit(1);
        }
        buf[n] = '\0';
        printf("  [Child  PID=%d] read(fd=%d) -> %zd bytes: \"%s\"\n",
               getpid(), pipefd[0], n, buf);
        close(pipefd[0]);
        printf("  [Child  PID=%d] close(fd=%d) -> success\n", getpid(), pipefd[0]);
        _exit(0);
    } else {
        /* Parent */
        close(pipefd[0]);
        const char *msg = "Hello via pipe() system call!";
        printf("  [Parent PID=%d] write(fd=%d) -> %zu bytes: \"%s\"\n",
               getpid(), pipefd[1], strlen(msg), msg);
        if (write(pipefd[1], msg, strlen(msg)) == -1) {
            perror("write");
        }
        close(pipefd[1]);
        printf("  [Parent PID=%d] close(fd=%d) -> success\n", getpid(), pipefd[1]);

        int status;
        wait(&status);
        printf("  [Parent PID=%d] wait() -> child %d exited, status %d\n",
               getpid(), pid, WEXITSTATUS(status));
    }

    /* mkfifo() */
    printf("\n  mkfifo(\"%s\", 0666)\n", FIFO_PATH);
    unlink(FIFO_PATH);
    if (mkfifo(FIFO_PATH, 0666) == -1) {
        perror("mkfifo");
        return;
    }
    printf("  FIFO created at: %s\n", FIFO_PATH);

    /* open() + close() */
    printf("\n  open(\"%s\", O_RDONLY)\n", FIFO_PATH);
    int fd = open(FIFO_PATH, O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        perror("open (read)");
    } else {
        printf("  fd = %d\n", fd);
        close(fd);
        printf("  close(%d) -> success\n", fd);
    }

    printf("\n  open(\"%s\", O_WRONLY)\n", FIFO_PATH);
    fd = open(FIFO_PATH, O_WRONLY | O_NONBLOCK);
    if (fd == -1) {
        perror("open (write)");
    } else {
        printf("  fd = %d\n", fd);
        close(fd);
        printf("  close(%d) -> success\n", fd);
    }

    unlink(FIFO_PATH);
}
