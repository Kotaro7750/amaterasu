CFLAGS = -Wall -Wextra -nostdinc -nostdlib -fno-builtin -fno-common -g
LDFLAGS = -Map app.map -s -T app.ld -x --nmagic
LIB = ../lib/lib.o
TARGET = test

SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)

$(TARGET):$(OBJ) $(LIB) 
	ld $(LDFLAGS) -o $@ $^

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f *~ *.o *.map $(TARGET)

.PHONY: clean
