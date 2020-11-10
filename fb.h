#ifndef _FB_H_
#define _FB_H_

#define BG_RED 0
#define BG_GREEN 0
#define BG_BLUE 255

struct Pixel {
  unsigned char B;
  unsigned char G;
  unsigned char R;
  unsigned char _reserved;
};

struct FrameBuffer {
  struct Pixel *Base;
  unsigned long long Size;
  unsigned int HorizontalResolution;
  unsigned int VerticalResolution;
};

void DrawPixel(struct FrameBuffer *fb, unsigned int x, unsigned int y,
               unsigned char R, unsigned char G, unsigned char B);
void DrawBox(struct FrameBuffer *fb, unsigned int x, unsigned int y,
             unsigned char R, unsigned char G, unsigned char B);

#endif
