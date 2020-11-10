#include "fb.h"

void DrawPixel(struct FrameBuffer *fb, unsigned int x, unsigned int y,
               unsigned char R, unsigned char G, unsigned char B) {
  struct Pixel* p = fb->Base + (fb->HorizontalResolution * y) + x;
  p->R = R;
  p->G = G;
  p->B = B;
}

void DrawBox(struct FrameBuffer *fb,unsigned int x,unsigned int y,unsigned char R,unsigned char G,unsigned char B){
  for (int i = 0; i<100; i++) {
    for (int j=0; j<100; j++) {
      DrawPixel(fb, x+i, y+j, R, G, B);
    }
  }
}
