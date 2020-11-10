#include "fb.h"
#include "graphic.h"

void start_kernel(void *_t __attribute__((unused)), struct FrameBuffer *fb,
                  void *_fs_start __attribute__((unused))) {

  FBInit(fb);
  SetBackGround(BG_RED, BG_GREEN, BG_BLUE);

  DrawBox(fb, 10, 10, 255, 0, 0);
  while (1)
    ;
}
