# Seal Shell - Custom Shell with Job Control and I/O Redirection
# Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -O2
DEBUGFLAGS = -g -O0 -DDEBUG
TARGET = seal

# Source files
SRCS = main.c lexer.c parser.c pipeline.c redirect.c jobs.c signals.c builtins.c utils.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Debug build
debug: CFLAGS = -Wall -Wextra -std=gnu11 $(DEBUGFLAGS)
debug: clean $(TARGET)

# Link
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile
%.o: %.c shell.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJS) $(TARGET)

# Test
test: $(TARGET)
	@echo "Running test suite..."
	@./tests/test_runner.sh

# Install
install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

# Uninstall
uninstall:
	rm -f /usr/local/bin/$(TARGET)

.PHONY: all clean test debug install uninstall
