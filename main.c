#include "include/fb.h"
#include "include/graphic.h"
#include "include/interrupt.h"
#include "include/kbc.h"
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

  EnableCPUInterrupt();

  while (1)
    CpuHalt();
}
