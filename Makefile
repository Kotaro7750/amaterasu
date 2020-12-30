TARGET = kernel.elf
BOOTLOADER = fs/EFI/BOOT/BOOTX64.EFI

CFLAGS = -Wall -Wextra -nostdinc -nostdlib -fno-builtin -fno-common -g
LDFLAGS = -Map kernel.map -x -T kernel.ld --nmagic

SRC = main.c fb.c graphic.c font.c x86_64.c kbc.c interrupt.c pic.c handler.c paging.c physicalMemory.c acpi.c util.c hpet.c
OBJ = $(SRC:%.c=%.o)

$(TARGET):$(OBJ)
	ld $(LDFLAGS) -o $@ $+

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

%.o: %.s
	gcc $(CFLAGS) -c -o $@ $<

$(BOOTLOADER):
	make -C metallica2
	cp ./metallica2/main.efi $(BOOTLOADER)

run: $(TARGET) $(BOOTLOADER)
	cp $(TARGET) ./fs/
	cp boot.conf ./fs/
	qemu-system-x86_64 -m 4G -bios OVMF.fd -hda fat:rw:fs -boot c -s

debug-run:$(TARGET) $(BOOTLOADER)
	cp $(TARGET) ./fs/
	cp boot.conf ./fs/
	qemu-system-x86_64 -m 4G -bios OVMF.fd -hda fat:rw:fs -boot c -d int -s -S -no-reboot -no-shutdown

clean:
	rm -f *~ *.o *.map $(BOOTLOADER) ./fs/$(TARGET) $(TARGET) include/*~

.PHONY: clean
