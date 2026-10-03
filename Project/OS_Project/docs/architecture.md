# Architecture Documentation

## Overview

The Linux IPC Monitoring and Message Transfer System is an integrated C application that demonstrates six Operating Systems course outcomes through a single, real message-transfer workflow. It is NOT a collection of separate demonstrations — it is one connected application where all COs contribute to the same message-processing pipeline.

## Design Principles

1. **Integration:** All six COs are demonstrated within one message-transfer workflow
2. **Real Operations:** Every advertised operation actually executes and is verifiable
3. **Error Handling:** All system calls check return values and use `perror()` on failures
4. **Clean Resource Management:** File descriptors closed, memory freed, children waited for, FIFOs unlinked
5. **Thread Safety:** Mutex-protected statistics with no data races
6. **Signal Safety:** Async-signal-safe handlers using `write()` instead of `printf()`

## Core Module: `ipc_session.c`

The integrated IPC session implements:

### Data Flow

```
User Input → malloc() buffer → fork() → Parent (Sender)
                                          ↓ write()
                                    Pipe/FIFO Buffer
                                          ↓ read()
                                    Child (Receiver) → Display
                                          ↓
                                    waitpid() → Cleanup
```

### CO Integration

| CO | Where Demonstrated | How |
|----|-------------------|-----|
| CO-1 | Every system call | Shows operation, return value, and user/kernel space transition |
| CO-2 | `fork()`, `waitpid()` | Real processes with PIDs, lifecycle, and synchronization |
| CO-3 | `pipe()`, `mkfifo()`, `sigaction()` | Two transport modes with real byte transfer and signals |
| CO-4 | `malloc()`, Copy-on-Write demo | Heap buffers and CoW variable demonstration |
| CO-5 | `stat()`, file descriptors | FIFO metadata, inode display, descriptor management |
| CO-6 | `pthread_mutex_t` | Mutex-protected statistics, no data races |

### Statistics Flow

```
Child Process                    Parent Process
     ↓                                ↓
read() messages              write() messages
     ↓                                ↓
stats_record_received()      stats_record_sent()
     ↓                                ↓
     └──────→ stats_pipe ←────────────┘
                  ↓
            Parent reads
            child stats
```

## Build Architecture

```
src/*.c → [GCC -c] → build/*.o → [GCC -pthread] → build/os_project
```

## Thread Safety

- Statistics protected by `pthread_mutex_t`
- Each thread receives its own argument structure (heap-allocated)
- `pthread_join()` ensures all threads complete before main continues
- No deadlocks: mutex always released after lock
