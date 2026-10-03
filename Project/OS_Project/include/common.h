#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#define FIFO_PATH "/tmp/os_project_fifo"
#define BUFFER_SIZE 256

/* Print a section header */
void print_header(const char *title);

/* Print a subsection header */
void print_subheader(const char *title);

/* Print a step in a flow diagram (printf-style) */
void print_step(const char *format, ...);

/* Print an arrow */
void print_arrow(void);

/* Pause and wait for user to press Enter */
void pause_output(void);

#endif /* COMMON_H */
