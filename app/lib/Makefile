CC = gcc
CFLAGS = -Wall -Wextra -nostdinc -nostdlib -fno-builtin -fno-common -g
AR = ar
SRCDIR = ./src
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:%.c=%.o)
TARGET = lib.o

$(TARGET): $(OBJS)
	$(AR) rvs $(TARGET) $^ 

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *~ $(TARGET) $(OBJS)

.PHONY: clean
