#include "co3_ipc.h"

/*
 * CO-3: IPC / SIGNALS
 *
 * System calls: pipe(), fork(), read(), write(), close(), wait()
 *               mkfifo(), open(), unlink()
 *               signal(), kill()
 */

/* ---- Signal handler for SIGUSR1 ---- */
static volatile sig_atomic_t sigusr1_received = 0;

void sigusr1_handler(int sig)
{
    (void)sig;
    sigusr1_received = 1;
    const char *msg = "  [Signal] SIGUSR1 received\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

/* ---- Signal handler for SIGINT ---- */
void sigint_handler(int sig)
{
    (void)sig;
    const char *msg = "\n  [Signal] SIGINT received\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    unlink(FIFO_PATH);
    _exit(0);
}

/* ---- PART A: Anonymous Pipe ---- */
static void part_a_anonymous_pipe(void)
{
    print_subheader("ANONYMOUS PIPE");

    int pipefd[2];
    printf("\n  pipe()\n");
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }
    printf("  read fd  = %d\n", pipefd[0]);
    printf("  write fd = %d\n", pipefd[1]);

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
        printf("  [Child  PID=%d] close(fd=%d)\n", getpid(), pipefd[1]);

        char buf[BUFFER_SIZE];
        printf("  [Child  PID=%d] read(fd=%d)\n", getpid(), pipefd[0]);
        ssize_t n = read(pipefd[0], buf, BUFFER_SIZE - 1);
        if (n == -1) {
            perror("read");
            close(pipefd[0]);
            _exit(1);
        }
        buf[n] = '\0';
        printf("  [Child  PID=%d] read() -> %zd bytes: \"%s\"\n",
               getpid(), n, buf);

        close(pipefd[0]);
        printf("  [Child  PID=%d] close(fd=%d)\n", getpid(), pipefd[0]);
        fflush(stdout);
        _exit(0);
    } else {
        /* Parent */
        close(pipefd[0]);
        printf("  [Parent PID=%d] close(fd=%d)\n", getpid(), pipefd[0]);

        const char *msg = "Hello from parent via anonymous pipe!";
        printf("  [Parent PID=%d] write(fd=%d): \"%s\"\n",
               getpid(), pipefd[1], msg);
        if (write(pipefd[1], msg, strlen(msg)) == -1) {
            perror("write");
        }

        close(pipefd[1]);
        printf("  [Parent PID=%d] close(fd=%d)\n", getpid(), pipefd[1]);

        int status;
        wait(&status);
        printf("  [Parent PID=%d] wait() -> child %d, status %d\n",
               getpid(), pid, WEXITSTATUS(status));
    }
}

/* ---- PART B: Named Pipe / FIFO ---- */
static void part_b_fifo(void)
{
    print_subheader("NAMED PIPE (FIFO)");

    unlink(FIFO_PATH);

    printf("\n  mkfifo(\"%s\", 0666)\n", FIFO_PATH);
    if (mkfifo(FIFO_PATH, 0666) == -1) {
        perror("mkfifo");
        return;
    }
    printf("  FIFO created: %s\n", FIFO_PATH);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        unlink(FIFO_PATH);
        return;
    }

    if (pid == 0) {
        /* Child = Reader */
        usleep(100000);
        printf("  [Reader  PID=%d] open(\"%s\", O_RDONLY)\n", getpid(), FIFO_PATH);
        int fd = open(FIFO_PATH, O_RDONLY);
        if (fd == -1) {
            perror("open (read)");
            _exit(1);
        }
        printf("  [Reader  PID=%d] fd = %d\n", getpid(), fd);

        char buf[BUFFER_SIZE];
        ssize_t n = read(fd, buf, BUFFER_SIZE - 1);
        if (n == -1) {
            perror("read");
            close(fd);
            _exit(1);
        }
        buf[n] = '\0';
        printf("  [Reader  PID=%d] read() -> %zd bytes: \"%s\"\n",
               getpid(), n, buf);

        close(fd);
        printf("  [Reader  PID=%d] close(%d)\n", getpid(), fd);
        fflush(stdout);
        _exit(0);
    } else {
        /* Parent = Writer */
        printf("  [Writer  PID=%d] open(\"%s\", O_WRONLY)\n", getpid(), FIFO_PATH);
        int fd = open(FIFO_PATH, O_WRONLY);
        if (fd == -1) {
            perror("open (write)");
            wait(NULL);
            unlink(FIFO_PATH);
            return;
        }
        printf("  [Writer  PID=%d] fd = %d\n", getpid(), fd);

        const char *msg = "Hello from writer via FIFO!";
        printf("  [Writer  PID=%d] write(fd=%d): \"%s\"\n",
               getpid(), fd, msg);
        if (write(fd, msg, strlen(msg)) == -1) {
            perror("write");
        }

        close(fd);
        printf("  [Writer  PID=%d] close(%d)\n", getpid(), fd);

        int status;
        wait(&status);
        printf("  [Writer  PID=%d] wait() -> child %d, status %d\n",
               getpid(), pid, WEXITSTATUS(status));
    }

    printf("\n  unlink(\"%s\")\n", FIFO_PATH);
    unlink(FIFO_PATH);
    printf("  FIFO removed.\n");
}

/* ---- PART C: Signals ---- */
static void part_c_signals(void)
{
    print_subheader("SIGNALS");

    printf("\n  signal(SIGUSR1, handler)\n");
    if (signal(SIGUSR1, sigusr1_handler) == SIG_ERR) {
        perror("signal");
        return;
    }

    printf("  signal(SIGINT, handler)\n");
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal");
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        /* Child */
        printf("  [Child  PID=%d] kill(parent, SIGUSR1)\n", getpid());
        sleep(1);
        if (kill(getppid(), SIGUSR1) == -1) {
            perror("kill");
        }
        sleep(1);
        printf("  [Child  PID=%d] _exit(0)\n", getpid());
        fflush(stdout);
        _exit(0);
    } else {
        /* Parent */
        printf("  [Parent PID=%d] Waiting for SIGUSR1...\n", getpid());
        while (!sigusr1_received) {
            pause();
        }
        printf("  [Parent PID=%d] SIGUSR1 received\n", getpid());

        int status;
        wait(&status);
        printf("  [Parent PID=%d] wait() -> child %d, status %d\n",
               getpid(), pid, WEXITSTATUS(status));
    }
}

/* ---- PART D: Synchronization ---- */
static void part_d_synchronization(void)
{
    print_subheader("SYNCHRONIZATION");

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid == 0) {
        /* Child: read two messages */
        close(pipefd[1]);

        for (int i = 0; i < 2; i++) {
            char buf[BUFFER_SIZE];
            ssize_t n = read(pipefd[0], buf, BUFFER_SIZE - 1);
            if (n <= 0) break;
            buf[n] = '\0';
            printf("  [Child]  Received message %d: \"%s\"\n", i + 1, buf);
        }

        close(pipefd[0]);
        _exit(0);
    } else {
        /* Parent: write two messages */
        close(pipefd[0]);

        const char *msg1 = "First synchronized message";
        const char *msg2 = "Second synchronized message";

        write(pipefd[1], msg1, strlen(msg1));
        printf("  [Parent] Sent message 1\n");

        write(pipefd[1], msg2, strlen(msg2));
        printf("  [Parent] Sent message 2\n");

        close(pipefd[1]);

        int status;
        wait(&status);
        printf("  [Parent] wait() -> child %d, status %d\n",
               pid, WEXITSTATUS(status));
    }
}

/* ---- Main CO-3 entry point ---- */
void co3_demo(void)
{
    print_header("CO-3: IPC / SIGNALS");

    part_a_anonymous_pipe();
    pause_output();

    part_b_fifo();
    pause_output();

    part_c_signals();
    pause_output();

    part_d_synchronization();
}
