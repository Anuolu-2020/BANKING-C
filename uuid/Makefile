# Compiler and flags
CC := gcc
CFLAGS := -O2 -g -Wall -Wshadow -pedantic -Werror -std=c99 -I.

# Directories
SRCDIR := .
BINDIR := .

# Target library
TARGET := libuuid.a

# Source files
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(SOURCES:.c=.o)

# Default target
all: $(TARGET)

# Rule to build the library
$(TARGET): $(OBJECTS)
	ar rcs $@ $^

# Rule to compile .c files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
