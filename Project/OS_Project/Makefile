# Linux IPC and Systems Programming Demonstrator
# Makefile

CC = gcc
CFLAGS = -Wall -Wextra -pthread -Iinclude
LDFLAGS = -pthread

SRCDIR = src
INCDIR = include
BINDIR = build

SOURCES = $(SRCDIR)/main.c \
          $(SRCDIR)/common.c \
          $(SRCDIR)/ipc_session.c \
          $(SRCDIR)/co1_system_calls.c \
          $(SRCDIR)/co2_process_control.c \
          $(SRCDIR)/co3_ipc.c \
          $(SRCDIR)/co4_memory.c \
          $(SRCDIR)/co5_files.c \
          $(SRCDIR)/co6_threads.c

OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BINDIR)/%.o)

TARGET = $(BINDIR)/os_project

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(BINDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BINDIR)

run: $(TARGET)
	./$(TARGET)
