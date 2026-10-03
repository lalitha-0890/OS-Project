#include "co5_files.h"

/*
 * CO-5: FILE SYSTEMS AND FILE I/O
 *
 * System calls used: mkfifo(), open(), read(), write(), close(), stat(), unlink()
 */

/* Helper: print file type from stat mode */
static void print_file_type(mode_t mode)
{
    if (S_ISREG(mode))       printf("Regular file\n");
    else if (S_ISDIR(mode))  printf("Directory\n");
    else if (S_ISFIFO(mode)) printf("FIFO (named pipe)\n");
    else if (S_ISLNK(mode))  printf("Symbolic link\n");
    else if (S_ISSOCK(mode)) printf("Socket\n");
    else                     printf("Unknown\n");
}

/* Helper: print permissions in rwx format */
static void print_permissions(mode_t mode)
{
    char perms[10] = "---------";
    if (mode & S_IRUSR) perms[0] = 'r';
    if (mode & S_IWUSR) perms[1] = 'w';
    if (mode & S_IXUSR) perms[2] = 'x';
    if (mode & S_IRGRP) perms[3] = 'r';
    if (mode & S_IWGRP) perms[4] = 'w';
    if (mode & S_IXGRP) perms[5] = 'x';
    if (mode & S_IROTH) perms[6] = 'r';
    if (mode & S_IWOTH) perms[7] = 'w';
    if (mode & S_IXOTH) perms[8] = 'x';
    printf("%s", perms);
}

void co5_demo(void)
{
    print_header("CO-5: FILESYSTEM DIAGNOSTIC");

    printf("\n  fd 0 = stdin, fd 1 = stdout, fd 2 = stderr\n");

    /* Create FIFO */
    unlink(FIFO_PATH);

    printf("\n  Calling mkfifo(\"%s\", 0666)...\n", FIFO_PATH);
    if (mkfifo(FIFO_PATH, 0666) == -1) {
        perror("mkfifo");
        return;
    }
    printf("  FIFO created.\n");

    /* stat() the FIFO */
    struct stat st;
    if (stat(FIFO_PATH, &st) == -1) {
        perror("stat");
        unlink(FIFO_PATH);
        return;
    }

    printf("\n  Path:        %s\n", FIFO_PATH);
    printf("  File type:   ");
    print_file_type(st.st_mode);
    printf("  Permissions: ");
    print_permissions(st.st_mode);
    printf("  Inode:       %ld\n", (long)st.st_ino);
    printf("  Size:        %ld bytes\n", (long)st.st_size);
    printf("  Links:       %ld\n", (long)st.st_nlink);

    /* Fork: parent = writer, child = reader */
    printf("\n  Forking: parent = writer, child = reader...\n");
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        unlink(FIFO_PATH);
        return;
    }

    if (pid == 0) {
        /* Child = Reader */
        printf("  [Reader Child PID=%d] Opening FIFO...\n", getpid());
        int fd_read = open(FIFO_PATH, O_RDONLY);
        if (fd_read == -1) {
            perror("open (read)");
            fflush(stdout);
            _exit(1);
        }
        printf("  [Reader Child PID=%d] fd = %d\n", getpid(), fd_read);

        char buf[BUFFER_SIZE];
        ssize_t n = read(fd_read, buf, BUFFER_SIZE - 1);
        if (n == -1) {
            perror("read");
        } else {
            buf[n] = '\0';
            printf("  [Reader Child PID=%d] read() = %zd bytes: \"%s\"\n",
                   getpid(), n, buf);
        }
        close(fd_read);
        printf("  [Reader Child PID=%d] File closed.\n", getpid());
        fflush(stdout);
        _exit(0);
    } else {
        /* Parent = Writer */
        printf("  [Writer Parent PID=%d] Opening FIFO...\n", getpid());
        int fd_write = open(FIFO_PATH, O_WRONLY);
        if (fd_write == -1) {
            perror("open (write)");
            wait(NULL);
            unlink(FIFO_PATH);
            return;
        }
        printf("  [Writer Parent PID=%d] fd = %d\n", getpid(), fd_write);

        const char *msg = "Data through FIFO!";
        printf("  [Writer Parent PID=%d] write(fd=%d, \"%s\", %zu)\n",
               getpid(), fd_write, msg, strlen(msg));
        if (write(fd_write, msg, strlen(msg)) == -1) {
            perror("write");
        }
        close(fd_write);
        printf("  [Writer Parent PID=%d] File closed.\n", getpid());

        /* Wait for reader */
        int status;
        wait(&status);
        printf("  [Writer Parent PID=%d] Child terminated, status = %d\n",
               getpid(), WEXITSTATUS(status));
    }

    /* Cleanup */
    printf("\n  Calling unlink(\"%s\")...\n", FIFO_PATH);
    if (unlink(FIFO_PATH) == -1) {
        perror("unlink");
    } else {
        printf("  FIFO removed successfully.\n");
    }
}
