# Compiler and flags
CC = gcc
CFLAGS = -Wall -Iinclude

# Directories
SRCDIR = src
BUILDDIR = build
INCDIR = include

# Source files
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))

# Target executable
TARGET = $(BUILDDIR)/httprouter

# Default target
all: $(TARGET)

# Link the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files to object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directory if it doesn't exist
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Clean up build artifacts
clean:
	rm -rf $(BUILDDIR)

# Phony targets
.PHONY: all clean