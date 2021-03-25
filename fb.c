#include <fb.h>

struct FrameBuffer fb;

void FBInit(struct FrameBuffer *_fb) {
  fb.Base = _fb->Base;
  fb.Size = _fb->Size;
  fb.HorizontalResolution = _fb->HorizontalResolution;
  fb.VerticalResolution = _fb->VerticalResolution;
}

void DrawPixel(unsigned int x, unsigned int y, unsigned char R, unsigned char G, unsigned char B) {
  struct Pixel *p = fb.Base + (fb.HorizontalResolution * y) + x;
  p->R = R;
  p->G = G;
  p->B = B;
}

void DrawBox(unsigned int x, unsigned int y, unsigned char R, unsigned char G, unsigned char B) {
  for (int i = 0; i < 100; i++) {
    for (int j = 0; j < 100; j++) {
      DrawPixel(x + i, y + j, R, G, B);
    }
  }
}
