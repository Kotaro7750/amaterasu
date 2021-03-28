TARGET = kernel.elf
TARGET_DEBUG = kernel.elf.debug
BOOTLOADER = fs/EFI/BOOT/BOOTX64.EFI

CFLAGS = -Wall -Wextra -nostdinc -nostdlib -fno-builtin -fno-common -g -I include
LDFLAGS = -Map kernel.map -s -x -T kernel.ld --nmagic
LDFLAGS_DEBUG = -Map kernel.map -x -T kernel.ld --nmagic

SRC = main.c fb.c graphic.c font.c x86_64.c kbc.c interrupt.c pic.c handler.c paging.c physicalMemory.c acpi.c util.c hpet.c scheduler.c syscall.c process.c ata.c fat.c kHeap.c file.c elf.c list.c queue.c
OBJ = $(SRC:%.c=%.o)

$(TARGET):$(OBJ)
	ld $(LDFLAGS) -o $@ $+

$(TARGET_DEBUG):$(OBJ)
	ld $(LDFLAGS_DEBUG) -o $@ $+

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

%.o: %.s
	gcc $(CFLAGS) -c -o $@ $<

$(BOOTLOADER):
	make -C metallica2
	cp ./metallica2/main.efi $(BOOTLOADER)

run: $(TARGET) $(BOOTLOADER)
	cp $(TARGET) ./fs/$(TARGET)
	cp boot.conf ./fs/
	qemu-system-x86_64 -m 4G -bios OVMF.fd -hda fat:rw:fs -boot c -s

debug-run:$(TARGET_DEBUG) $(BOOTLOADER)
	cp $(TARGET_DEBUG) ./fs/$(TARGET)
	cp boot.conf ./fs/
	qemu-system-x86_64 -m 4G -bios OVMF.fd -hda fat:rw:fs -boot c -d int -s -S -no-reboot -no-shutdown

clean:
	rm -f *~ *.o *.map $(BOOTLOADER) ./fs/$(TARGET) $(TARGET) ./fs/$(TARGET_DEBUG) $(TARGET_DEBUG) include/*~

.PHONY: clean
