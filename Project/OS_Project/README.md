# Linux IPC Monitoring and Message Transfer System

## Project Title

**Linux IPC Monitoring and Message Transfer System**

An integrated Linux application that demonstrates Inter-Process Communication using anonymous pipes and named FIFOs, while showcasing process management, system calls, memory management, filesystem resources, signals, and thread synchronization — all within one connected message-transfer workflow.

---

## Problem Statement

Operating Systems concepts like process management, inter-process communication, memory management, and file systems are often taught theoretically. This project provides a **hands-on, integrated** implementation where all six Course Outcomes (CO-1 to CO-6) are demonstrated within a single, real message-transfer application — not as separate toy programs.

---

## Objectives

1. Build one integrated application that transfers real messages between processes
2. Demonstrate the OS as a service layer through actual system calls with visible return values
3. Show real process creation, PIDs, lifecycle, and synchronization
4. Implement working anonymous pipe and named FIFO transport modes
5. Visualize memory management and Copy-on-Write behavior
6. Connect FIFO implementation with Linux file-system concepts (inode, file descriptors)
7. Demonstrate thread race conditions and mutex-based synchronization
8. Provide controlled signal handling and clean resource management

---

## CO Mapping

| CO | Module | Concepts | Key System Calls |
|----|--------|----------|------------------|
| CO-1 | `ipc_session.c` | User space, Kernel space, System calls, OS service layer | `pipe()`, `fork()`, `read()`, `write()`, `open()`, `close()`, `mkfifo()` |
| CO-2 | `ipc_session.c` | Process abstraction, fork, PIDs, lifecycle, synchronization | `fork()`, `getpid()`, `getppid()`, `waitpid()`, `_exit()` |
| CO-3 | `ipc_session.c` | Anonymous pipes, Named pipes (FIFOs), Signals, Synchronization | `pipe()`, `mkfifo()`, `open()`, `read()`, `write()`, `close()`, `sigaction()`, `kill()` |
| CO-4 | `ipc_session.c` | Address space, Stack, Heap, Data, Text, Copy-on-Write, malloc/free | `fork()`, `malloc()`, `free()` |
| CO-5 | `ipc_session.c` | File descriptors, inode, FIFO special file, stat, file types | `mkfifo()`, `open()`, `read()`, `write()`, `close()`, `stat()`, `unlink()` |
| CO-6 | `ipc_session.c` | POSIX threads, Race condition, Mutex, Critical section | `pthread_create()`, `pthread_join()`, `pthread_mutex_lock()`, `pthread_mutex_unlock()` |

---

## Architecture

```
OS_Project/
│
├── README.md
├── Makefile
│
├── include/
│   ├── common.h              (shared utilities)
│   ├── ipc_session.h         (integrated IPC session — CORE)
│   ├── co1_system_calls.h    (supporting: system-call verification)
│   ├── co2_process_control.h (supporting: process control demo)
│   ├── co3_ipc.h             (supporting: IPC concepts demo)
│   ├── co4_memory.h          (supporting: memory diagnostic)
│   ├── co5_files.h           (supporting: filesystem diagnostic)
│   └── co6_threads.h         (supporting: concurrency diagnostic)
│
├── src/
│   ├── main.c                (menu system)
│   ├── common.c              (utility functions)
│   ├── ipc_session.c         (CORE: integrated message-transfer system)
│   ├── co1_system_calls.c    (supporting module)
│   ├── co2_process_control.c (supporting module)
│   ├── co3_ipc.c             (supporting module)
│   ├── co4_memory.c          (supporting module)
│   ├── co5_files.c           (supporting module)
│   └── co6_threads.c         (supporting module)
│
├── docs/
│   ├── architecture.md
│   ├── system_calls.md
│   └── demonstration.md
│
└── tests/
    └── test_script.sh
```

---

## Technologies

- **Language:** C (C99/C11)
- **Platform:** Linux / Ubuntu (WSL compatible)
- **Compiler:** GCC
- **APIs:** POSIX system calls, POSIX Threads (pthreads)
- **No external frameworks or libraries**

---

## Compilation Instructions

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install gcc make
```

### Build

```bash
cd OS_Project
make
```

This creates the executable at `build/os_project`.

### Clean

```bash
make clean
```

---

## Execution Instructions

### Run the program

```bash
./build/os_project
```

### Menu Options

```
===========================================
  LINUX IPC MONITORING & MESSAGE TRANSFER
===========================================

  1. Start integrated IPC session — anonymous pipe
  2. Start integrated IPC session — named FIFO
  3. Run complete end-to-end demonstration
  4. Inspect IPC resources and file descriptors
  5. Run memory/Copy-on-Write diagnostic
  6. Run concurrency and mutex diagnostic
  7. Run system-call verification instructions
  8. Exit safely

Choose an option:
```

---

## Module Explanations

### Integrated IPC Session (`ipc_session.c`) — CORE

This is the main application. It implements a real end-to-end message-transfer workflow:

**Mode A: Anonymous Pipe**
- Creates a real anonymous pipe with `pipe()`
- Forks a child process (receiver) from the parent (sender)
- Transfers actual user-entered messages byte-by-byte through the pipe
- Uses newline delimiters to separate messages
- Child sends SIGUSR1 to parent as ready notification
- Parent waits for child with `waitpid()`
- Statistics tracked with mutex protection

**Mode B: Named FIFO**
- Creates a real FIFO with `mkfifo()` at `/tmp/os_project_fifo`
- Displays FIFO metadata with `stat()` (inode, type, permissions)
- Forks reader and writer processes
- Transfers real messages through the FIFO
- Properly cleans up with `unlink()`

**Signals**
- SIGUSR1: Child-to-parent ready notification
- SIGINT: Controlled shutdown
- Uses `sigaction()` for safe signal handling
- Async-signal-safe handlers (no printf in handlers)

**Memory Management**
- Message buffers allocated with `malloc()`
- Copy-on-Write demonstration with controlled variable
- Proper `free()` during cleanup

**Concurrency**
- Mutex-protected statistics (messages sent/received, bytes, errors)
- No data races on shared state

---

### Supporting Diagnostics (Options 4-7)

These provide focused demonstrations of individual CO concepts:

- **Option 4:** File descriptors, FIFO creation, `stat()` metadata, inode display
- **Option 5:** Process address space, Copy-on-Write, `malloc()`/`free()`
- **Option 6:** Race condition (no mutex) vs. mutex solution
- **Option 7:** System-call verification instructions and `strace` commands

---

## Important System Calls

| System Call | Purpose |
|-------------|---------|
| `pipe()` | Create anonymous pipe |
| `fork()` | Create child process |
| `read()` | Read from file descriptor |
| `write()` | Write to file descriptor |
| `open()` | Open file/FIFO |
| `close()` | Close file descriptor |
| `mkfifo()` | Create named pipe (FIFO) |
| `unlink()` | Remove file/directory entry |
| `waitpid()` | Wait for child termination |
| `getpid()` | Get process ID |
| `getppid()` | Get parent process ID |
| `sigaction()` | Install signal handler |
| `kill()` | Send signal to process |
| `stat()` | Get file metadata |
| `malloc()` | Allocate heap memory |
| `free()` | Free heap memory |
| `pthread_create()` | Create thread |
| `pthread_join()` | Wait for thread |
| `pthread_mutex_lock()` | Lock mutex |
| `pthread_mutex_unlock()` | Unlock mutex |

---

## Expected Output

### Anonymous Pipe Session

```
===========================================
  INTEGRATED IPC SESSION — ANONYMOUS PIPE
===========================================

--- CO-1/CO-5: Creating Anonymous Pipe ---
  pipe() returned: pipefd[0] = 3 (read end)
                    pipefd[1] = 4 (write end)

--- CO-2: Forking Child Process ---
  [Parent PID=4903] Child created with PID=4904
  [Child  PID=4904] Message 1 received (17 bytes): "Hello from parent"
  [Child  PID=4904] Message 2 received (14 bytes): "Second message"

  +----------------------------------------+
  |        SESSION STATISTICS              |
  +----------------------------------------+
  | Messages sent:      2                  |
  | Messages received:  2                  |
  | Bytes transferred:  31                 |
  | Errors:             0                  |
  +----------------------------------------+
```

### Named FIFO Session

```
===========================================
  INTEGRATED IPC SESSION — NAMED PIPE (FIFO)
===========================================

--- CO-5: Creating Named Pipe (FIFO) ---
  mkfifo() succeeded — FIFO created at /tmp/os_project_fifo
  stat() results:
    File type:    FIFO (named pipe)
    Inode number: 4154
    Permissions:  644

  [Reader Child PID=4944] Message 1 received (14 bytes): "Hello via FIFO"
  [Writer Parent PID=4943] FIFO removed successfully.
```

---

## Viva Questions

1. **What is a system call?**
   A system call is a controlled entry point into the kernel that allows a user-space program to request a service from the OS.

2. **What happens during `fork()`?**
   The kernel creates a new process (child) that is a copy of the calling process (parent). Both continue execution from the same point.

3. **What is Copy-on-Write?**
   After `fork()`, parent and child share the same physical memory pages. The kernel only creates a separate copy when one process modifies a page.

4. **What is the difference between an anonymous pipe and a FIFO?**
   An anonymous pipe has no filesystem pathname and can only be used between related processes. A FIFO has a pathname and can be used between unrelated processes.

5. **What is a zombie process?**
   A zombie is a process that has terminated but whose exit status has not been read by the parent (via `waitpid()`).

6. **What is a race condition?**
   A race condition occurs when multiple threads access shared data concurrently, and the final result depends on the order of execution.

7. **How does a mutex solve the race condition?**
   A mutex ensures mutual exclusion: only one thread can hold the mutex at a time, so only one thread can execute the critical section.

8. **What is an inode?**
   An inode is a data structure on disk that stores metadata about a file (type, permissions, size, owner, etc.).

9. **Why is a FIFO visible in the filesystem but an anonymous pipe is not?**
   A FIFO has a directory entry (name → inode mapping), while an anonymous pipe exists only as a pair of file descriptors in the kernel.

10. **What are the standard file descriptors?**
    0 = stdin, 1 = stdout, 2 = stderr.

---

## Demonstration Sequence

1. **Compile:** `make`
2. **Run:** `./build/os_project`
3. **Select option 1** for anonymous pipe session
4. **Enter messages** and observe real transfer
5. **Select option 2** for FIFO session
6. **Run `ls -l /tmp/os_project_fifo`** in another terminal to see the FIFO
7. **Select option 3** for complete end-to-end demonstration

---

## strace Verification

```bash
# Trace all relevant system calls
strace -f -e trace=pipe,pipe2,fork,clone,read,write,openat,close,mkfifo,wait4 ./build/os_project

# Inspect FIFO (before cleanup)
ls -l /tmp/os_project_fifo
stat /tmp/os_project_fifo
```

---

## Limitations

- The zombie demonstration is disabled by default (enable in `co2_process_control.c`)
- CO-6 race condition may not always show incorrect results (depends on timing)
- Address values shown in CO-4 vary due to ASLR
- This is an educational demonstration, not a production system
