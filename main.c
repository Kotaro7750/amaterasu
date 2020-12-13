#include "include/fb.h"
#include "include/graphic.h"
#include "include/interrupt.h"
#include "include/kbc.h"
#include "include/paging.h"
#include "include/physicalMemory.h"
#include "include/pic.h"
#include "include/x86_64.h"

void start_kernel(void *_t __attribute__((unused)), struct FrameBuffer *_fb, struct PhysicalMemoryFreeMapInfo *freeMapInfo) {

  FBInit(_fb);
  gdtInit();
  GraphicInit();

  idtInit();
  picInit();
  KBCInit();

  PagingInit();
  PhysicalMemoryManagementInit(*freeMapInfo);

  EnableCPUInterrupt();

  unsigned long long cr3 = GetCR3();
  struct L4PTEntry *pt4 = (struct L4PTEntry *)cr3;

  unsigned long long addr = AllocateSinglePageFrame();
  puth(addr);
  puts("\n");

  ((struct L4PTEntry *)addr)[0] = pt4[0];
  asm volatile("mov %[value], %%cr3" ::[value] "r"(addr));

  while (1)
    CpuHalt();
}
