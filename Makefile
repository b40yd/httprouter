# Makefile
CC = gcc

CFLAGS = -std=c99 -Wall -g

SRCS = httprouter.c \
hr_tree.c \
hr_string.c \
hr_palloc.c \
hr_array.c \
utils.c

OBJS = $(SRCS:.c=.o)

TARGET = httprouter

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)