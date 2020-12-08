#include "include/paging.h"
#include "include/graphic.h"
#include "include/interrupt.h"
#include "include/x86_64.h"

void PageFaultHandlerASM(void);

void PagingInit(void) {
  void *handler;
  asm volatile("lea PageFaultHandlerASM, %[handler]"
               : [ handler ] "=r"(handler));
  SetInterruptDescriptor(PAGE_FAULT_EXCP_NUM, handler, 1);
}

void PageFaultHandler(void) {
  unsigned long long value;
  asm volatile("mov %%cr2, %[value]" : [ value ] "=a"(value));

  puts("PAGE FAULT\nVIRTUAL ADDR ");
  puth(value);
  while (1) {
    CpuHalt();
  }
}

void CausePageFoult(void) {
  // addr >= 4GB, so this page is not mapped
  unsigned long long *invalidAddr = 0xffffffffff;
  *invalidAddr = 42;
}

unsigned long long CalcPhyAddr(unsigned long long LinearAddr) {
  unsigned long long physicalAddr;

  unsigned int l4ptIndex = (LinearAddr >> 39) & 0x1ff;
  unsigned int l3ptIndex = (LinearAddr >> 30) & 0x1ff;
  unsigned int l2ptIndex = (LinearAddr >> 21) & 0x1ff;
  unsigned int l1ptIndex = (LinearAddr >> 12) & 0x1ff;

  if (0x7fffffffffff < LinearAddr && LinearAddr < 0xffff800000000000) {
    return 0;
  }

  unsigned long long cr3 = GetCR3();

  struct L4PTEntry *l4ptBase = (struct L4PTEntry *)cr3;
  struct L4PTEntry l4ptEntry = l4ptBase[l4ptIndex];
  if (l4ptEntry.Present != 1) {
    return 0;
  }

  struct L3PTEntry *l3ptBase =
      (struct L3PTEntry *)(l4ptEntry.L3PTPhysAddr << 12);
  struct L3PTEntry l3ptEntry = l3ptBase[l3ptIndex];
  if (l3ptEntry.Present != 1) {
    return 0;
  }

  // 1GB Page
  if (l3ptEntry.PageSize == 1) {
    struct L3PTEntry1GB *l3ptBase1GB =
        (struct L3PTEntry1GB *)(l4ptEntry.L3PTPhysAddr << 12);
    struct L3PTEntry1GB l3ptEntry1GB = l3ptBase1GB[l3ptIndex];

    // Present flag is already checked
    unsigned int inPageOffset = LinearAddr & 0x1fffffff;
    unsigned long long physicalPageAddr = l3ptEntry1GB.PageFramePhysAddr << 30;
    physicalAddr = physicalPageAddr + inPageOffset;

  } else {
    // 2MB or 4KB Page
    struct L2PTEntry *l2ptBase =
        (struct L2PTEntry *)(l3ptEntry.L2PTPhysAddr << 12);
    struct L2PTEntry l2ptEntry = l2ptBase[l2ptIndex];
    if (l2ptEntry.Present != 1) {
      return 0;
    }

    // 2MB Page
    if (l2ptEntry.PageSize == 1) {
      struct L2PTEntry2MB *l2ptBase2MB =
          (struct L2PTEntry2MB *)(l3ptEntry.L2PTPhysAddr << 12);
      struct L2PTEntry2MB l2ptEntry2MB = l2ptBase2MB[l2ptIndex];

      // Present flag is already checked
      unsigned int inPageOffset = LinearAddr & 0xfffff;
      unsigned long long physicalPageAddr = l2ptEntry2MB.PageFramePhysAddr
                                            << 21;
      physicalAddr = physicalPageAddr + inPageOffset;

    } else {
      // 4KB Page
      struct L1PTEntry *l1ptBase =
          (struct L1PTEntry *)(l2ptEntry.L1PTPhysAddr << 12);
      struct L1PTEntry l1ptEntry = l1ptBase[l1ptIndex];

      if (l1ptEntry.Present != 1) {
        return 0;
      }

      unsigned int inPageOffset = LinearAddr & 0xfff;
      unsigned long long physicalPageAddr = l1ptEntry.PageFramePhysAddr << 12;
      physicalAddr = physicalPageAddr + inPageOffset;
    }
  }

  return physicalAddr;
}

void _dumpPTEntry(struct L2PTEntry l1ptEntry) {
  puts("P ");
  puth(l1ptEntry.Present);
  puts("\n");

  puts("RW ");
  puth(l1ptEntry.ReadWrite);
  puts("\n");

  puts("US ");
  puth(l1ptEntry.UserSupervisor);
  puts("\n");

  puts("SZ ");
  puth(l1ptEntry.PageSize);
  puts("\n");

  puts("ADDR ");
  puth(l1ptEntry.L1PTPhysAddr);
  puts("\n");
}
