# System Calls Reference

## Process Management

| Call | Description | Module |
|------|-------------|--------|
| `fork()` | Create a child process | CO-1, CO-2, CO-3, CO-4 |
| `getpid()` | Get current process ID | CO-2, CO-3 |
| `getppid()` | Get parent process ID | CO-2, CO-3 |
| `wait()` | Wait for child termination | CO-1, CO-2, CO-3 |
| `_exit()` | Terminate current process | CO-2, CO-3, CO-4 |

## Inter-Process Communication

| Call | Description | Module |
|------|-------------|--------|
| `pipe()` | Create anonymous pipe | CO-1, CO-3 |
| `mkfifo()` | Create named pipe (FIFO) | CO-1, CO-3, CO-5 |
| `open()` | Open file/FIFO | CO-1, CO-3, CO-5 |
| `read()` | Read from file descriptor | CO-1, CO-3, CO-5 |
| `write()` | Write to file descriptor | CO-1, CO-3, CO-5 |
| `close()` | Close file descriptor | CO-1, CO-3, CO-5 |
| `unlink()` | Remove filesystem entry | CO-3, CO-5 |

## Signals

| Call | Description | Module |
|------|-------------|--------|
| `signal()` | Install signal handler | CO-3 |
| `kill()` | Send signal to process | CO-3 |

## File System

| Call | Description | Module |
|------|-------------|--------|
| `stat()` | Get file metadata | CO-5 |

## Memory Management

| Call | Description | Module |
|------|-------------|--------|
| `malloc()` | Allocate heap memory | CO-4, CO-6 |
| `free()` | Free heap memory | CO-4, CO-6 |

## Threads (POSIX)

| Call | Description | Module |
|------|-------------|--------|
| `pthread_create()` | Create a new thread | CO-6 |
| `pthread_join()` | Wait for thread termination | CO-6 |
| `pthread_mutex_lock()` | Lock mutex | CO-6 |
| `pthread_mutex_unlock()` | Unlock mutex | CO-6 |
