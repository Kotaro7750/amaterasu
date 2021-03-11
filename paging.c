#include "include/paging.h"
#include "include/graphic.h"
#include "include/interrupt.h"
#include "include/x86_64.h"

void PageFaultHandlerASM(void);

void PagingInit(void) {
  void *handler;
  asm volatile("lea PageFaultHandlerASM, %[handler]" : [ handler ] "=r"(handler));
  SetInterruptDescriptor(PAGE_FAULT_EXCP_NUM, handler, 1);

  // enable 0~5GB-1
  struct L3PTEntry1GB *l3ptBase = (struct L3PTEntry1GB *)AllocateSinglePageFrame();
  for (int i = 0; i < 512; i++) {
    if (i < 5) {
      l3ptBase[i].Present = 1;
      l3ptBase[i].ReadWrite = 1;
      l3ptBase[i].UserSupervisor = 0;
      l3ptBase[i].PageWriteThrough = 0;
      l3ptBase[i].PageCacheDisable = 0;
      l3ptBase[i].Accessed = 0;
      l3ptBase[i].Dirty = 0;
      l3ptBase[i].PageSize = 1;
      l3ptBase[i].Global = 0;
      l3ptBase[i]._ignored = 0;
      l3ptBase[i].PAT = 0;
      l3ptBase[i].Reserved = 0;
      l3ptBase[i].PageFramePhysAddr = ((unsigned long long)((unsigned long long)i * 0x40000000) >> 30);
      l3ptBase[i]._ignored3 = 0;
      l3ptBase[i].ProtectionKey = 0;
      l3ptBase[i].ExecuteDisable = 0;
    } else {
      l3ptBase[i].Present = 0;
    }
  }
  struct L4PTEntry *l4ptBase = (struct L4PTEntry *)AllocateSinglePageFrame();
  for (int i = 0; i < 512; i++) {
    if (i == 0) {
      l4ptBase[i].Present = 1;
      l4ptBase[i].ReadWrite = 1;
      l4ptBase[i].UserSupervisor = 0;
      l4ptBase[i].PageWriteThrough = 0;
      l4ptBase[i].PageCacheDisable = 0;
      l4ptBase[i].Accessed = 0;
      l4ptBase[i]._ignored1 = 0;
      l4ptBase[i].PageSize = 0;
      l4ptBase[i]._ignored2 = 0;
      l4ptBase[i].L3PTPhysAddr = ((unsigned long long)(l3ptBase) >> 12);
      l4ptBase[i]._ignored3 = 0;
      l4ptBase[i].ExecuteDisable = 0;
    } else {
      l4ptBase[i].Present = 0;
    }
  }

  asm volatile("mov %[value], %%cr3" ::[value] "r"(l4ptBase));
}

void PageFaultHandler(void) {
  unsigned long long virtualAddress;
  asm volatile("mov %%cr2, %[virtualAddress]" : [ virtualAddress ] "=a"(virtualAddress));

  unsigned long long errorCode;
  unsigned long long rbp;
  asm volatile("mov %%rbp, %[Rbp]" : [ Rbp ] "=r"(rbp));
  errorCode = *(unsigned long long *)(rbp + 8 * 7 + 16);

  char present = errorCode & 0x1;
  char write = (errorCode & 0x2) >> 1;
  char user = (errorCode & 0x4) >> 2;
  char reservedWrite = (errorCode & 0x8) >> 3;
  char iFetch = (errorCode & 0x16) >> 4;

  puts("PAGE FAULT\nVIRTUAL ADDR ");
  puth(virtualAddress);

  puts("\nPRESENT ");
  puth(present);
  puts("\nWRITE ");
  puth(write);
  puts("\nUSER ");
  puth(user);
  puts("\nRWRITE ");
  puth(reservedWrite);
  puts("\nIFETCH ");
  puth(iFetch);

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

  unsigned int l4ptIndex = L4_PT_INDEX(LinearAddr);
  unsigned int l3ptIndex = L3_PT_INDEX(LinearAddr);
  unsigned int l2ptIndex = L2_PT_INDEX(LinearAddr);
  unsigned int l1ptIndex = L1_PT_INDEX(LinearAddr);

  if (0x7fffffffffff < LinearAddr && LinearAddr < 0xffff800000000000) {
    puts("NOT CANONICAL\n");
    return 0;
  }

  unsigned long long cr3 = GetCR3();

  struct L4PTEntry *l4ptBase = (struct L4PTEntry *)cr3;
  struct L4PTEntry l4ptEntry = l4ptBase[l4ptIndex];
  if (l4ptEntry.Present != 1) {
    puts("NOT PRESENT 4\n");
    return 0;
  }

  struct L3PTEntry *l3ptBase = (struct L3PTEntry *)(l4ptEntry.L3PTPhysAddr << 12);
  struct L3PTEntry l3ptEntry = l3ptBase[l3ptIndex];
  if (l3ptEntry.Present != 1) {
    puts("\nNOT PRESENT 3\n");
    return 0;
  }

  // 1GB Page
  if (l3ptEntry.PageSize == 1) {
    struct L3PTEntry1GB *l3ptBase1GB = (struct L3PTEntry1GB *)(l4ptEntry.L3PTPhysAddr << 12);
    struct L3PTEntry1GB l3ptEntry1GB = l3ptBase1GB[l3ptIndex];

    // Present flag is already checked
    unsigned int inPageOffset = LinearAddr & 0x3fffffff;
    unsigned long long physicalPageAddr = (unsigned long long)(l3ptEntry1GB.PageFramePhysAddr) << 30;
    physicalAddr = physicalPageAddr + inPageOffset;

  } else {
    // 2MB or 4KB Page
    struct L2PTEntry *l2ptBase = (struct L2PTEntry *)(l3ptEntry.L2PTPhysAddr << 12);
    struct L2PTEntry l2ptEntry = l2ptBase[l2ptIndex];
    if (l2ptEntry.Present != 1) {
      puts("NOT PRESENT 2\n");
      return 0;
    }

    // 2MB Page
    if (l2ptEntry.PageSize == 1) {
      struct L2PTEntry2MB *l2ptBase2MB = (struct L2PTEntry2MB *)(l3ptEntry.L2PTPhysAddr << 12);
      struct L2PTEntry2MB l2ptEntry2MB = l2ptBase2MB[l2ptIndex];

      // Present flag is already checked
      unsigned int inPageOffset = LinearAddr & 0x1fffff;
      unsigned long long physicalPageAddr = (unsigned long long)(l2ptEntry2MB.PageFramePhysAddr) << 21;
      physicalAddr = physicalPageAddr + inPageOffset;

    } else {
      // 4KB Page
      struct L1PTEntry *l1ptBase = (struct L1PTEntry *)(l2ptEntry.L1PTPhysAddr << 12);
      struct L1PTEntry l1ptEntry = l1ptBase[l1ptIndex];
      if (l1ptEntry.Present != 1) {
        puts("NOT PRESENT 1\n");
        return 0;
      }

      unsigned int inPageOffset = LinearAddr & 0xfff;
      unsigned long long physicalPageAddr = (unsigned long long)(l1ptEntry.PageFramePhysAddr) << 12;
      physicalAddr = physicalPageAddr + inPageOffset;
    }
  }

  return physicalAddr;
}

void _dumpPTEntry(struct L2PTEntry2MB l1ptEntry) {
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
  puth(l1ptEntry.PageFramePhysAddr);
  puts("\n");
}

void _dumpPageTableInfo() {
  unsigned long long cr3 = GetCR3();
  struct L4PTEntry *l4ptBase = (struct L4PTEntry *)cr3;
  struct L4PTEntry l4ptEntry = l4ptBase[0];

  struct L3PTEntry *l3ptBase = (struct L3PTEntry *)(l4ptEntry.L3PTPhysAddr << 12);
  struct L3PTEntry l3ptEntry = l3ptBase[0];

  struct L2PTEntry2MB *l2ptBase = (struct L2PTEntry *)(l3ptEntry.L2PTPhysAddr << 12);
  struct L2PTEntry2MB l2ptEntry = l2ptBase[0];

  puts(":RW ");
  puth(l2ptEntry.ReadWrite);
  puts(",");

  puts("US ");
  puth(l2ptEntry.UserSupervisor);
  puts(",");

  puts("PWT ");
  puth(l2ptEntry.PageWriteThrough);
  puts("\n");

  puts("PCD ");
  puth(l2ptEntry.PageCacheDisable);
  puts(",");

  puts("SZ ");
  puth(l2ptEntry.PageSize);
  puts(",");

  puts("G ");
  puth(l2ptEntry.Global);
  puts(",");

  puts("PAT ");
  puth(l2ptEntry.PAT);
  puts("\n");

  puts("ADDR ");
  puth(l2ptEntry.PageFramePhysAddr);
  puts(",");

  puts("PROTECTION ");
  puth(l2ptEntry.ProtectionKey);
  puts(",");

  puts("ED ");
  puth(l2ptEntry.ExecuteDisable);
  puts("\n");
}
