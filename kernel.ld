OUTPUT_FORMAT(elf64-x86-64)
ENTRY(start_kernel)

MEMORY
{
  ram(rwx) : ORIGIN = 0x1300000, LENGTH = 1m
}

SECTIONS
{
        .text : {
                *(.text)
        }>ram AT>ram

        .rodata : {
                *(.rodata)
        }>ram AT>ram

        .data : {
                *(.data)
        }>ram AT>ram

        .bss : {
                *(.bss)
        }>ram AT>ram

        .eh_frame : {
                *(.eh_frame)
        }>ram AT>ram

        .debug_info : {
                *(.debug_info)
        }
}
