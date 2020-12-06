#include "include/fb.h"
#include "include/graphic.h"
#include "include/kbc.h"
#include "include/x86_64.h"

void start_kernel(void *_t __attribute__((unused)), struct FrameBuffer *_fb,
                  void *_fs_start __attribute__((unused))) {

  FBInit(_fb);
  gdtInit();
  GraphicInit();

  // puts("ABCDE\nFG");
  while (1) {
    char c = getc();
    if (('a' <= c) && (c <= 'z'))
      c = c - 'a' + 'A';
    else if (c == '\n')
      putc('\r');
    putc(c);
  }
  while (1)
    ;
}
