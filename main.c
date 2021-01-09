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
#include "include/syscall.h"
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

void taskB() {
  while (1) {
    putc('B');
    volatile unsigned long long wait = 10000000;
    while (wait--)
      ;
  }
}

void taskC() {
  while (1) {
    putc('C');
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
  SyscallInit();
  SchedulerInit();

  PhysicalMemoryManagementInit(*freeMapInfo);
  PagingInit();

  EnableCPUInterrupt();

  Syscall(SYSCALL_EXEC, (unsigned long long)taskA, 0, 0);
  Syscall(SYSCALL_EXEC, (unsigned long long)taskB, 0, 0);
  Syscall(SYSCALL_EXEC, (unsigned long long)taskC, 0, 0);

  // Schedule(0);
  SchedulerStart();

  while (1)
    CpuHalt();
}
