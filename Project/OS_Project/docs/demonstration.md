# Demonstration Guide

## Quick Start

```bash
cd OS_Project
make
./build/os_project
```

## Demonstrating to Faculty

### Option 1: Anonymous Pipe Session

**What to show:**
- Real `pipe()` call returning fd[0] and fd[1]
- `fork()` creating parent (sender) and child (receiver)
- User-entered messages transferred byte-by-byte
- Child displays received messages with its PID
- `waitpid()` preventing zombies
- Mutex-protected statistics

**Key talking point:** "Every message you see was actually written by the parent and read by the child through a real kernel pipe buffer."

---

### Option 2: Named FIFO Session

**What to show:**
- `mkfifo()` creating a visible filesystem entry
- `stat()` showing real inode number, file type, and permissions
- Fork-based reader/writer processes
- Real message transfer through the FIFO
- Proper `unlink()` cleanup

**Key talking point:** "The FIFO has a real directory entry — run `ls -l /tmp/os_project_fifo` in another terminal to see it."

---

### Option 3: Complete End-to-End Demonstration

Runs the full workflow:
1. Anonymous pipe session
2. Named FIFO session
3. Memory/Copy-on-Write diagnostic
4. Concurrency/mutex diagnostic
5. System-call verification summary

---

### Option 4: IPC Resources and File Descriptors

**What to show:**
- Standard file descriptors (0, 1, 2)
- `stat()` output with real inode numbers
- FIFO vs regular file vs directory
- Why FIFO is visible but anonymous pipe is not

---

### Option 5: Memory/Copy-on-Write Diagnostic

**What to show:**
- Address space layout (STACK, HEAP, DATA, TEXT)
- Real addresses from `&variable`
- After fork(): child modifies, parent unchanged
- Copy-on-Write explanation

---

### Option 6: Concurrency and Mutex Diagnostic

**What to show:**
- Race condition: expected vs actual counter (may differ)
- Mutex solution: always consistent results
- Critical section protection

---

## Verification Commands

```bash
# Trace system calls
strace -f -e trace=pipe,pipe2,fork,clone,read,write,openat,close,mkfifo,wait4 ./build/os_project

# Inspect FIFO (run during option 2)
ls -l /tmp/os_project_fifo
stat /tmp/os_project_fifo

# Check for zombie processes
ps aux | grep Z

# Memory leak check
valgrind --leak-check=full ./build/os_project
```
