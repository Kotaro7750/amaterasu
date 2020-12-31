#include "include/acpi.h"
#include "include/fb.h"
#include "include/graphic.h"
#include "include/hpet.h"
#include "include/interrupt.h"
#include "include/kbc.h"
#include "include/paging.h"
#include "include/physicalMemory.h"
#include "include/pic.h"
#include "include/scheduler.h"
#include "include/x86_64.h"

struct __attribute__((packed)) PlatformInfo {
  struct FrameBuffer fb;
  void *RSDPAddress;
};

void taskA() {
  while (1) {
    putc('A');
    volatile unsigned long long wait = 10000000;
    while (wait--)
      ;
  }
}

void start_kernel(void *_t __attribute__((unused)), struct PlatformInfo *_pi, struct PhysicalMemoryFreeMapInfo *freeMapInfo) {

  FBInit(&(_pi->fb));
  gdtInit();
  GraphicInit();

  idtInit();
  picInit();
  KBCInit();
  ACPIInit(_pi->RSDPAddress);
  HPETInit();

  PagingInit();
  PhysicalMemoryManagementInit(*freeMapInfo);

  EnableCPUInterrupt();
  SchedulerInit();

  //unsigned long long cr3 = GetCR3();
  //struct L4PTEntry *pt4 = (struct L4PTEntry *)cr3;

  //unsigned long long addr = AllocateSinglePageFrame();
  //puth(addr);
  //puts("\n");

  //((struct L4PTEntry *)addr)[0] = pt4[0];
  //asm volatile("mov %[value], %%cr3" ::[value] "r"(addr));
  taskA();

  while (1)
    CpuHalt();
}
