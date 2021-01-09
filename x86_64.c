#include "include/x86_64.h"
#include "include/physicalMemory.h"

unsigned char
CALC_ACCESS_BYTE(unsigned char Pr, unsigned char Priv, unsigned char S, unsigned char Ex, unsigned char DC, unsigned char RW, unsigned char Ac) {
  unsigned char AccessByte = 0;
  AccessByte |= 0x1 & Ac;
  AccessByte |= 0x2 & (RW << 1);
  AccessByte |= 0x4 & (DC << 2);
  AccessByte |= 0x8 & (Ex << 3);
  AccessByte |= 0x10 & (S << 4);
  AccessByte |= 0x60 & (Priv << 5);
  AccessByte |= 0x80 & (Pr << 7);

  return AccessByte;
};

unsigned char CALC_FLAGS(unsigned char Gr, unsigned char Sz, unsigned char L) {
  unsigned char Flags = 0;
  Flags |= 0x2 & (L << 1);
  Flags |= 0x4 & (Sz << 2);
  Flags |= 0x8 & (Gr << 3);

  return Flags;
};

unsigned long long CalcGDTEntry(unsigned long long base, unsigned long long limit, unsigned long long AccessByte, unsigned long long Flags) {
  unsigned long long gdtEntry = 0;
  gdtEntry |= limit & 0xffff;
  gdtEntry |= (base & 0xffff) << 16;
  gdtEntry |= ((base >> 16) & 0xff) << 32;
  gdtEntry |= (AccessByte & 0xff) << 40;
  gdtEntry |= ((limit >> 16) & 0xf) << 48;
  gdtEntry |= (Flags & 0xf) << 52;
  gdtEntry |= ((base >> 24) & 0xff) << 56;

  return gdtEntry;
}

unsigned long long CalcTSSDescriptorHigher(unsigned long long base) {
  unsigned long long higher = 0;
  higher += (base >> 32) & 0xffffffff;
  return higher;
}

unsigned long long gdt[7];
unsigned long long gdtr[2];
unsigned int tss[26];

void gdtInit() {
  // setup gdt
  gdt[0] = GDT_NULL;
  gdt[1] = GDT_KERNEL_CODE;
  gdt[2] = GDT_KERNEL_DATA;
  gdt[3] = GDT_USER_CODE;
  gdt[4] = GDT_USER_DATA;
  for (int i = 0; i < 26; i++) {
    tss[i] = 0;
  }
  tss[1] = AllocateSinglePageFrame() + PAGE_SIZE - 1;
  gdt[5] = GDT_TSS_DESCRIPTOR_LOWER((unsigned long long)(&(tss[0])));
  gdt[6] = GDT_TSS_DESCRIPTOR_HIGHER((unsigned long long)(&(tss[0])));

  // load gdt to gdtr
  gdtr[0] = ((unsigned long long)gdt << 16) | ((sizeof(gdt) - 1) & 0xffff);
  gdtr[1] = (unsigned long long)gdt >> 48;
  asm volatile("lgdt gdtr");

  // load segment selector to DS,SS
  // asm volatile("movw $2*8, %ax\n"
  asm volatile("movw $2*0, %ax\n"
               "movw %ax, %ds\n"
               "movw %ax, %ss\n");

  // load segment selector to CS using lretq
  // lretq will return to the instruction where dummyLabel indicates.
  // this technique is used to change CS in a safe way.
  unsigned short segmentSelector = 8;
  unsigned long long dummy;
  asm volatile("pushq %[selector];"
               "leaq dummyLabel(%%rip), %[dummy];"
               "pushq %[dummy];"
               "lretq;"
               "dummyLabel:"
               : [ dummy ] "=r"(dummy)
               : [ selector ] "m"(segmentSelector));

  unsigned short tssSelector = 40;
  asm volatile("ltr %[tssSelector]" ::[tssSelector] "m"(tssSelector));
}

void EnableCPUInterrupt(void) { asm volatile("sti"); }

unsigned long long GetCR3(void) {
  unsigned long long cr3;
  asm volatile("mov %%cr3, %[value]" : [ value ] "=r"(cr3));
  return cr3;
}

unsigned char InByte(unsigned short addr) {
  unsigned char data;
  asm volatile("in %[addr], %[data]" : [ data ] "=a"(data) : [ addr ] "d"(addr));
  return data;
}

void OutByte(unsigned short addr, unsigned char data) { asm volatile("out %[data], %[addr]" : : [ addr ] "d"(addr), [ data ] "a"(data)); }

void CpuHalt(void) { asm volatile("hlt"); }
