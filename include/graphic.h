#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_
#include <fb.h>

#define UNUM64_STR_LEN 16

void FBInit(struct FrameBuffer *_fb);
void GraphicInit();
void SetBackGround(unsigned char R, unsigned char G, unsigned char B);
void putc(unsigned char c);
void puts(unsigned char *str);
void puth(unsigned long long num);

#endif
