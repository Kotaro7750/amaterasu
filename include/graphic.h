#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_
#include "fb.h"

void FBInit(struct FrameBuffer *_fb);
void GraphicInit();
void SetBackGround(unsigned char R, unsigned char G, unsigned char B);
void putc(unsigned char c);
void puts(unsigned char *str);

#endif
