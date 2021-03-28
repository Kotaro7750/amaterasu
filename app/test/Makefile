CFLAGS = -Wall -Wextra -nostdinc -nostdlib -fno-builtin -fno-common -g -I ../lib/include
LDFLAGS = -Map app.map -s -T app.ld -x --nmagic
LDFLAGS_DEBUG = -Map app.map -T app.ld -x --nmagic
LIB = ../lib/lib.o
TARGET = test.app
TARGET_DEBUG = $(TARGET).debug

SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)

$(TARGET):$(OBJ) $(LIB) 
	ld $(LDFLAGS) -o $@ $^

$(TARGET_DEBUG):$(OBJ) $(LIB) 
	ld $(LDFLAGS_DEBUG) -o $@ $^

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f *~ *.o *.map $(TARGET) $(TARGET_DEBUG)

cp:
	cp $(TARGET) ../../fs/test

.PHONY: clean,cp
