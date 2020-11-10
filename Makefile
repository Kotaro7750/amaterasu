TARGET = kernel.bin
BOOTLOADER = fs/EFI/BOOT/BOOTX64.EFI
CFLAGS = -Wall -Wextra -nostdinc -nostdlib -fno-builtin -fno-common 
LDFLAGS = -Map kernel.map -s -x -T kernel.ld

$(TARGET):main.o fb.o graphic.o
	ld $(LDFLAGS) -o $@ $+

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

%.o: %.s
	gcc $(CFLAGS) -c -o $@ $<

$(BOOTLOADER):
	make -C metallica2
	cp ./metallica2/main.efi $(BOOTLOADER)

run:$(TARGET) $(BOOTLOADER)
	cp $(TARGET) ./fs/
	cp boot.conf ./fs/
	qemu-system-x86_64 -m 4G -bios OVMF.fd -hda fat:rw:fs -boot c

clean:
	rm -f *~ *.o *.map ./fs/$(TARGET) $(TARGET) include/*~

.PHONY: clean
