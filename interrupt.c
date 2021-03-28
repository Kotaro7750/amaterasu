/**
 * @file interrupt.c
 * @brief 割り込み関連の処理
 */

#include <graphic.h>
#include <interrupt.h>
#include <syscall.h>
#include <x86_64.h>

/**
 * @brief IDTの実体
 */
struct InterruptDescriptor idt[MAX_INTR_NUM];

/**
 * @brief IDTRの実体
 */
unsigned long long idtr[2];

void DefaultHandler(void);
void DoubleFaultHandlerASM(void);
void GeneralProtectionFaultHandlerASM(void);

void SetInterruptDescriptor(unsigned char interruptNumber, void *handler, unsigned int present) {
  idt[interruptNumber].Offset0_15 = (unsigned long long)handler;
  // TODO this magic number should be MACRO.
  // kernel code segment
  idt[interruptNumber].SegmentSelector = CS_SEGMENT_SELECTOR_KERNEL;
  idt[interruptNumber].IST = 0;
  idt[interruptNumber]._zero1 = 0;
  // TODO this magic number should be MACRO
  // 32bit interrupt gate
  idt[interruptNumber].Type = 0xe;
  idt[interruptNumber]._zero2 = 0;
  // TODO this magic number should be MACRO
  if (interruptNumber == SYSCALL_INTERRUPT_NUM) {
    idt[interruptNumber].DPL = 3;
  } else {
    idt[interruptNumber].DPL = 0;
  }
  idt[interruptNumber].P = present;
  idt[interruptNumber].Offset16_31 = (unsigned long long)handler >> 16;
  idt[interruptNumber].Offset32_63 = (unsigned long long)handler >> 32;
}

void idtInit() {
  void *handler;
  asm volatile("lea DefaultHandler, %[handler]" : [ handler ] "=r"(handler));

  for (int i = 0; i < MAX_INTR_NUM; i++) {
    SetInterruptDescriptor(i, handler, 0);
  }

  asm volatile("lea DoubleFaultHandlerASM, %[handler]" : [ handler ] "=r"(handler));
  SetInterruptDescriptor(DOUBLE_FAULT_EXCP_NUM, handler, 1);

  asm volatile("lea GeneralProtectionFaultHandlerASM, %[handler]" : [ handler ] "=r"(handler));
  SetInterruptDescriptor(GENERAL_PROTECTION_FAULT_EXCP_NUM, handler, 1);

  // load idt to idtr
  idtr[0] = ((unsigned long long)idt << 16) | ((sizeof(idt) - 1) & 0xffff);
  idtr[1] = (unsigned long long)idt >> 48;
  asm volatile("lidt idtr");
}

void DoubleFaultHandler(void) {
  puts("DOUBLE FAULT\n");
  while (1) {
    CpuHalt();
  }
}

void GeneralProtectionFaultHandler(unsigned long long rsp) {
  unsigned long long errorCode = *(unsigned long long *)rsp;

  char external = errorCode & 0x1;
  char tbl = (errorCode >> 1) & 0x3;
  short index = (errorCode >> 3) & 0x1fff;

  puts("GENERAL PROTECTION FAULT\n");

  puts("EXTERNAL: ");
  puth(external);
  puts("\n");

  puts("TBL: ");
  puth(tbl);
  puts("\n");

  puts("INDEX: ");
  puth(index);
  puts("\n");

  puts("RIP: ");
  puth(*(unsigned long long *)(rsp + 8));
  puts("\n");

  puts("CS: ");
  puth(*(unsigned long long *)(rsp + 16));
  puts("\n");

  puts("RFLAGS: ");
  puth(*(unsigned long long *)(rsp + 24));
  puts("\n");

  puts("RSP: ");
  puth(*(unsigned long long *)(rsp + 32));
  puts("\n");

  unsigned long long prevRsp = *(unsigned long long *)(rsp + 32);

  for (int i = 0; i < 5; i++) {
    puth(*(unsigned long long *)prevRsp);
    puts("\n");
    prevRsp -= 8;
  }

  while (1) {
    CpuHalt();
  }
}
