#include "include/interrupt.h"

struct InterruptDescriptor idt[MAX_INTR_NUM];
unsigned long long idtr[2];

void DefaultHandler(void);

void SetInterruptDescriptor(unsigned char interruptNumber, void* handler, unsigned int present){
  idt[interruptNumber].Offset0_15 = (unsigned long long)handler;
  // TODO this magic number should be MACRO.
  // kernel code segment
  idt[interruptNumber].SegmentSelector = 8;
  idt[interruptNumber].IST = 0;
  idt[interruptNumber]._zero1 = 0;
  // TODO this magic number should be MACRO
  // 32bit interrupt gate
  idt[interruptNumber].Type = 0xe;
  idt[interruptNumber]._zero2 = 0;
  // TODO this magic number should be MACRO
  idt[interruptNumber].DPL = 0;
  idt[interruptNumber].P = present;
  idt[interruptNumber].Offset16_31 = (unsigned long long)handler >> 16;
  idt[interruptNumber].Offset32_63 = (unsigned long long)handler >> 32;
}

void idtInit(){
  for (int i = 0; i < MAX_INTR_NUM; i++) {
    SetInterruptDescriptor(i, DefaultHandler,0);
  }

  // load idt to idtr
  idtr[0] = ((unsigned long long)idt << 16) | ((sizeof(idt) - 1) & 0xffff);
  idtr[1] = (unsigned long long)idt >> 48;
  asm volatile("lidt idtr");
}
