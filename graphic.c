#include "fb.h"

struct FrameBuffer fb;

void FBInit(struct FrameBuffer *_fb) {
  fb.Base = _fb->Base;
  fb.Size = _fb->Size;
  fb.HorizontalResolution = _fb->HorizontalResolution;
  fb.VerticalResolution = _fb->VerticalResolution;
}

void SetBackGround(unsigned char R, unsigned char G, unsigned char B) {
  for (int x = 0; x < fb.HorizontalResolution; x++) {
    for (int y = 0; y < fb.VerticalResolution; y++) {
      DrawPixel(&fb, x, y, R, G, B);
    }
  }
}
