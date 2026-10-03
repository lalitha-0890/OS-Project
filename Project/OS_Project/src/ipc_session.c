#include "ipc_session.h"

/*
 * ============================================================================
 * LINUX IPC MONITORING AND MESSAGE TRANSFER SYSTEM
 * ============================================================================
 *
 * Integrated message-transfer system demonstrating all six Course Outcomes
 * in one connected workflow.
 *
 * ============================================================================
 */

/* ============================================================================
 * SHARED STATE (protected by mutex)
 * ============================================================================ */

typedef struct {
    pthread_mutex_t mutex;
    long messages_sent;
    long messages_received;
    long bytes_transferred;
    long errors;
} session_stats_t;

static session_stats_t stats;

static void stats_init(void)
{
    memset(&stats, 0, sizeof(stats));
    if (pthread_mutex_init(&stats.mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        exit(1);
    }
}

static void stats_destroy(void)
{
    pthread_mutex_destroy(&stats.mutex);
}

static void stats_record_sent(long bytes)
{
    pthread_mutex_lock(&stats.mutex);
    stats.messages_sent++;
    stats.bytes_transferred += bytes;
    pthread_mutex_unlock(&stats.mutex);
}

static void stats_record_received(long bytes)
{
    pthread_mutex_lock(&stats.mutex);
    stats.messages_received++;
    stats.bytes_transferred += bytes;
    pthread_mutex_unlock(&stats.mutex);
}

static void stats_record_error(void)
{
    pthread_mutex_lock(&stats.mutex);
    stats.errors++;
    pthread_mutex_unlock(&stats.mutex);
}

static void stats_print(void)
{
    pthread_mutex_lock(&stats.mutex);
    printf("\n  +----------------------------------------+\n");
    printf("  |        SESSION STATISTICS              |\n");
    printf("  +----------------------------------------+\n");
    printf("  | Messages sent:      %ld\n", stats.messages_sent);
    printf("  | Messages received:  %ld\n", stats.messages_received);
    printf("  | Bytes transferred:  %ld\n", stats.bytes_transferred);
    printf("  | Errors:             %ld\n", stats.errors);
    printf("  +----------------------------------------+\n");
    pthread_mutex_unlock(&stats.mutex);
}

/* ============================================================================
 * SIGNAL HANDLING
 * ============================================================================ */

static volatile sig_atomic_t sigusr1_flag = 0;
static volatile sig_atomic_t sigint_flag = 0;

static void sigusr1_handler(int sig)
{
    (void)sig;
    sigusr1_flag = 1;
    const char *msg = "  [Signal] SIGUSR1 received\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

static void sigint_handler(int sig)
{
    (void)sig;
    sigint_flag = 1;
    const char *msg = "\n  [Signal] SIGINT received\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

static void setup_signal_handlers(void)
{
    struct sigaction sa_usr1, sa_int;

    memset(&sa_usr1, 0, sizeof(sa_usr1));
    sa_usr1.sa_handler = sigusr1_handler;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1) {
        perror("sigaction SIGUSR1");
    }

    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("sigaction SIGINT");
    }
}

/* ============================================================================
 * MESSAGE INPUT
 * ============================================================================ */

static char *read_user_message(void)
{
    printf("\n  Enter a message to send (max %d chars): ", BUFFER_SIZE - 1);
    fflush(stdout);

    char *buf = malloc(BUFFER_SIZE);
    if (buf == NULL) {
        perror("malloc");
        return NULL;
    }

    if (fgets(buf, BUFFER_SIZE, stdin) == NULL) {
        free(buf);
        return NULL;
    }

    /* Remove trailing newline */
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
        len--;
    }

    return buf;
}

/* ============================================================================
 * ANONYMOUS PIPE SESSION
 * ============================================================================ */

void ipc_session_pipe(void)
{
    print_header("IPC SESSION — ANONYMOUS PIPE");

    setup_signal_handlers();
    stats_init();

    printf("\n  Parent PID : %d\n", getpid());
    printf("  Parent PPID: %d\n", getppid());

    /* Create pipe */
    int pipefd[2];
    printf("\n  pipe()\n");
    if (pipe(pipefd) == -1) {
        perror("pipe");
        stats_record_error();
        return;
    }
    printf("  read fd  = %d\n", pipefd[0]);
    printf("  write fd = %d\n", pipefd[1]);

    /* Create stats pipe */
    int stats_pipe[2];
    if (pipe(stats_pipe) == -1) {
        perror("pipe (stats)");
        close(pipefd[0]);
        close(pipefd[1]);
        stats_record_error();
        return;
    }

    /* Copy-on-Write */
    int cow_var = 100;
    printf("\n  Before fork: cow_var = %d at %p\n", cow_var, (void *)&cow_var);

    /* Fork */
    printf("\n  fork()\n");
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        close(stats_pipe[0]);
        close(stats_pipe[1]);
        stats_record_error();
        return;
    }

    if (pid == 0) {
        /* === CHILD PROCESS (Receiver) === */
        printf("  [Child  PID=%d] PPID=%d\n", getpid(), getppid());

        cow_var = 200;
        printf("  [Child  PID=%d] cow_var = %d (modified)\n", getpid(), cow_var);

        close(pipefd[1]);
        printf("  [Child  PID=%d] close(fd=%d)\n", getpid(), pipefd[1]);

        close(stats_pipe[0]);

        printf("  [Child  PID=%d] kill(parent, SIGUSR1)\n", getpid());
        if (kill(getppid(), SIGUSR1) == -1) {
            perror("kill");
        }

        printf("  [Child  PID=%d] read(fd=%d)\n", getpid(), pipefd[0]);

        char *recv_buf = malloc(BUFFER_SIZE);
        if (recv_buf == NULL) {
            perror("malloc");
            close(pipefd[0]);
            close(stats_pipe[1]);
            _exit(1);
        }

        int msg_count = 0;
        char *line = malloc(BUFFER_SIZE);
        if (line == NULL) {
            perror("malloc");
            free(recv_buf);
            close(pipefd[0]);
            close(stats_pipe[1]);
            _exit(1);
        }
        size_t line_pos = 0;

        while (1) {
            char ch;
            ssize_t n = read(pipefd[0], &ch, 1);
            if (n == -1) {
                perror("read");
                stats_record_error();
                break;
            }
            if (n == 0) {
                printf("  [Child  PID=%d] Pipe EOF\n", getpid());
                break;
            }
            if (ch == '\n') {
                line[line_pos] = '\0';
                if (line_pos > 0) {
                    msg_count++;
                    printf("  [Child  PID=%d] Message %d: \"%s\" (%zu bytes)\n",
                           getpid(), msg_count, line, line_pos);
                    stats_record_received(line_pos);
                }
                line_pos = 0;
            } else {
                if (line_pos < BUFFER_SIZE - 1) {
                    line[line_pos++] = ch;
                }
            }
        }

        free(recv_buf);
        free(line);
        close(pipefd[0]);
        printf("  [Child  PID=%d] close(fd=%d)\n", getpid(), pipefd[0]);

        /* Write statistics back to parent */
        long child_received = 0;
        pthread_mutex_lock(&stats.mutex);
        child_received = stats.messages_received;
        pthread_mutex_unlock(&stats.mutex);

        if (write(stats_pipe[1], &child_received, sizeof(child_received)) == -1) {
            perror("write (stats)");
        }
        close(stats_pipe[1]);

        fflush(stdout);
        _exit(0);
    } else {
        /* === PARENT PROCESS (Sender) === */
        printf("  [Parent PID=%d] fork() -> child PID %d\n", getpid(), pid);
        printf("  [Parent PID=%d] cow_var = %d (unchanged)\n", getpid(), cow_var);

        close(pipefd[0]);
        printf("  [Parent PID=%d] close(fd=%d)\n", getpid(), pipefd[0]);

        close(stats_pipe[1]);

        printf("  [Parent PID=%d] Waiting for SIGUSR1...\n", getpid());
        while (!sigusr1_flag && !sigint_flag) {
            pause();
        }
        printf("  [Parent PID=%d] SIGUSR1 received\n", getpid());

        printf("\n  [Parent PID=%d] Enter messages (Ctrl+D to finish):\n", getpid());

        int msg_count = 0;
        while (!sigint_flag) {
            char *msg = read_user_message();
            if (msg == NULL) {
                break;
            }

            if (strlen(msg) == 0) {
                free(msg);
                continue;
            }

            msg_count++;
            printf("  [Parent PID=%d] write(fd=%d): \"%s\" (%zu bytes)\n",
                   getpid(), pipefd[1], msg, strlen(msg));

            ssize_t written = write(pipefd[1], msg, strlen(msg));
            if (written == -1) {
                perror("write");
                stats_record_error();
            } else {
                stats_record_sent(written);
            }
            if (write(pipefd[1], "\n", 1) == -1) {
                perror("write (newline)");
            }

            free(msg);
        }

        printf("\n  [Parent PID=%d] close(fd=%d)\n", getpid(), pipefd[1]);
        close(pipefd[1]);

        printf("  [Parent PID=%d] waitpid(%d)\n", getpid(), pid);
        int status;
        pid_t result = waitpid(pid, &status, 0);
        if (result == -1) {
            perror("waitpid");
            stats_record_error();
        } else {
            printf("  [Parent PID=%d] Child %d terminated, status %d\n",
                   getpid(), result, WEXITSTATUS(status));
        }

        /* Read child statistics */
        long child_received = 0;
        ssize_t n = read(stats_pipe[0], &child_received, sizeof(child_received));
        if (n == sizeof(child_received)) {
            pthread_mutex_lock(&stats.mutex);
            stats.messages_received = child_received;
            pthread_mutex_unlock(&stats.mutex);
            printf("  [Parent PID=%d] Child reported %ld messages received\n",
                   getpid(), child_received);
        }
        close(stats_pipe[0]);

        printf("  [Parent PID=%d] Session complete.\n", getpid());
    }

    stats_print();
}

/* ============================================================================
 * NAMED FIFO SESSION
 * ============================================================================ */

void ipc_session_fifo(void)
{
    print_header("IPC SESSION — NAMED PIPE (FIFO)");

    setup_signal_handlers();
    stats_init();

    printf("\n  Parent PID : %d\n", getpid());
    printf("  Parent PPID: %d\n", getppid());

    /* Create FIFO */
    unlink(FIFO_PATH);

    printf("\n  mkfifo(\"%s\", 0666)\n", FIFO_PATH);
    if (mkfifo(FIFO_PATH, 0666) == -1) {
        perror("mkfifo");
        stats_record_error();
        return;
    }
    printf("  FIFO created: %s\n", FIFO_PATH);

    /* stat() the FIFO */
    struct stat st;
    if (stat(FIFO_PATH, &st) == -1) {
        perror("stat");
        unlink(FIFO_PATH);
        stats_record_error();
        return;
    }

    printf("  File type:   %s\n", S_ISFIFO(st.st_mode) ? "FIFO" : "Unknown");
    printf("  Inode:       %ld\n", (long)st.st_ino);
    printf("  Permissions: %o\n", st.st_mode & 0777);

    /* Fork */
    printf("\n  fork()\n");
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        unlink(FIFO_PATH);
        stats_record_error();
        return;
    }

    if (pid == 0) {
        /* === CHILD PROCESS (FIFO Reader) === */
        printf("  [Reader Child PID=%d] PPID=%d\n", getpid(), getppid());

        printf("  [Reader Child PID=%d] kill(parent, SIGUSR1)\n", getpid());
        if (kill(getppid(), SIGUSR1) == -1) {
            perror("kill");
        }

        printf("  [Reader Child PID=%d] open(\"%s\", O_RDONLY)\n", getpid(), FIFO_PATH);
        int fd_read = open(FIFO_PATH, O_RDONLY);
        if (fd_read == -1) {
            perror("open (read)");
            fflush(stdout);
            _exit(1);
        }
        printf("  [Reader Child PID=%d] fd = %d\n", getpid(), fd_read);

        char *recv_buf = malloc(BUFFER_SIZE);
        if (recv_buf == NULL) {
            perror("malloc");
            close(fd_read);
            _exit(1);
        }

        int msg_count = 0;
        while (1) {
            ssize_t n = read(fd_read, recv_buf, BUFFER_SIZE - 1);
            if (n == -1) {
                perror("read");
                stats_record_error();
                break;
            }
            if (n == 0) {
                printf("  [Reader Child PID=%d] FIFO EOF\n", getpid());
                break;
            }
            recv_buf[n] = '\0';
            msg_count++;
            printf("  [Reader Child PID=%d] Message %d: \"%s\" (%zd bytes)\n",
                   getpid(), msg_count, recv_buf, n);
            stats_record_received(n);
        }

        free(recv_buf);
        close(fd_read);
        printf("  [Reader Child PID=%d] close(%d)\n", getpid(), fd_read);
        fflush(stdout);
        _exit(0);
    } else {
        /* === PARENT PROCESS (FIFO Writer) === */
        printf("  [Writer Parent PID=%d] fork() -> child PID %d\n", getpid(), pid);

        printf("  [Writer Parent PID=%d] Waiting for SIGUSR1...\n", getpid());
        while (!sigusr1_flag && !sigint_flag) {
            pause();
        }
        printf("  [Writer Parent PID=%d] SIGUSR1 received\n", getpid());

        printf("  [Writer Parent PID=%d] open(\"%s\", O_WRONLY)\n", getpid(), FIFO_PATH);
        int fd_write = open(FIFO_PATH, O_WRONLY);
        if (fd_write == -1) {
            perror("open (write)");
            waitpid(pid, NULL, 0);
            unlink(FIFO_PATH);
            stats_record_error();
            return;
        }
        printf("  [Writer Parent PID=%d] fd = %d\n", getpid(), fd_write);

        printf("\n  [Writer Parent PID=%d] Enter messages (Ctrl+D to finish):\n", getpid());

        int msg_count = 0;
        while (!sigint_flag) {
            char *msg = read_user_message();
            if (msg == NULL) {
                break;
            }

            if (strlen(msg) == 0) {
                free(msg);
                continue;
            }

            msg_count++;
            printf("  [Writer Parent PID=%d] write(fd=%d): \"%s\" (%zu bytes)\n",
                   getpid(), fd_write, msg, strlen(msg));

            ssize_t written = write(fd_write, msg, strlen(msg));
            if (written == -1) {
                perror("write");
                stats_record_error();
            } else {
                stats_record_sent(written);
            }

            free(msg);
        }

        printf("\n  [Writer Parent PID=%d] close(%d)\n", getpid(), fd_write);
        close(fd_write);

        printf("  [Writer Parent PID=%d] waitpid(%d)\n", getpid(), pid);
        int status;
        pid_t result = waitpid(pid, &status, 0);
        if (result == -1) {
            perror("waitpid");
            stats_record_error();
        } else {
            printf("  [Writer Parent PID=%d] Child %d terminated, status %d\n",
                   getpid(), result, WEXITSTATUS(status));
        }

        printf("  [Writer Parent PID=%d] unlink(\"%s\")\n", getpid(), FIFO_PATH);
        if (unlink(FIFO_PATH) == -1) {
            perror("unlink");
            stats_record_error();
        } else {
            printf("  [Writer Parent PID=%d] FIFO removed.\n", getpid());
        }

        printf("  [Writer Parent PID=%d] Session complete.\n", getpid());
    }

    stats_print();
}

/* ============================================================================
 * COMPLETE END-TO-END DEMONSTRATION
 * ============================================================================ */

void ipc_session_complete(void)
{
    print_header("COMPLETE END-TO-END IPC DEMONSTRATION");

    stats_init();
    setup_signal_handlers();

    printf("\n\n");
    printf("###########################################\n");
    printf("#  PART 1: ANONYMOUS PIPE SESSION         #\n");
    printf("###########################################\n");
    ipc_session_pipe();

    printf("\n\n");
    printf("###########################################\n");
    printf("#  PART 2: NAMED FIFO SESSION             #\n");
    printf("###########################################\n");
    ipc_session_fifo();

    printf("\n\n");
    printf("###########################################\n");
    printf("#  PART 3: MEMORY DIAGNOSTIC              #\n");
    printf("###########################################\n");
    extern void co4_demo(void);
    co4_demo();

    printf("\n\n");
    printf("###########################################\n");
    printf("#  PART 4: CONCURRENCY DIAGNOSTIC         #\n");
    printf("###########################################\n");
    extern void co6_demo(void);
    co6_demo();

    printf("\n\n");
    printf("###########################################\n");
    printf("#  PART 5: SYSTEM-CALL VERIFICATION       #\n");
    printf("###########################################\n");
    printf("\n  strace -f -e trace=pipe,pipe2,fork,clone,read,write,openat,close,mkfifo,wait4 ./build/os_project\n");
    printf("\n  ls -l %s\n", FIFO_PATH);
    printf("  stat %s\n", FIFO_PATH);

    stats_print();
    stats_destroy();

    printf("\n###########################################\n");
    printf("#  COMPLETE DEMONSTRATION FINISHED        #\n");
    printf("###########################################\n");
}
