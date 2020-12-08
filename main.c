#include "include/fb.h"
#include "include/graphic.h"
#include "include/interrupt.h"
#include "include/kbc.h"
#include "include/paging.h"
#include "include/pic.h"
#include "include/x86_64.h"

void start_kernel(void *_t __attribute__((unused)), struct FrameBuffer *_fb,
                  void *_fs_start __attribute__((unused))) {

  FBInit(_fb);
  gdtInit();
  GraphicInit();

  idtInit();
  picInit();
  KBCInit();

  PagingInit();

  EnableCPUInterrupt();

  unsigned long long cr3 = GetCR3();
  puth(cr3);
  puts("\n");
  struct L4PTEntry *pt4 = (struct L4PTEntry *)cr3;

  unsigned long long dummy[512];
  unsigned long long addr;
  for (int i = 0; i < 512; i++) {
    addr = &(dummy[i]);
    if ((addr & 0xfff) == 0) {
      break;
    }
  }

  ((struct L4PTEntry *)addr)[0] = pt4[0];
  asm volatile("mov %[value], %%cr3" ::[value] "r"(addr));

  while (1)
    CpuHalt();
}
