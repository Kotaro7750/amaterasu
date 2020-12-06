#include "include/graphic.h"
#include "include/fb.h"
#include "include/font.h"

unsigned int cursorX;
unsigned int cursorY;

void GraphicInit() { 
  SetBackGround(BG_RED, BG_GREEN, BG_BLUE); 
  cursorX = 0;
  cursorY = 0;
}

void SetBackGround(unsigned char R, unsigned char G, unsigned char B) {
  for (unsigned int x = 0; x < fb.HorizontalResolution; x++) {
    for (unsigned int y = 0; y < fb.VerticalResolution; y++) {
      DrawPixel(x, y, R, G, B);
    }
  }
}

// TODO process when exceed the limit defined by resolution
void putc(unsigned char c) {
  if (c == '\n') {
    cursorX = 0;
    cursorY += FONT_HEIGHT;
  } else {
    for (unsigned int x = 0; x < FONT_WIDTH; x++) {
      for (unsigned int y = 0; y < FONT_HEIGHT; y++) {
        if (fontBitMap[c][y][x]) {
          DrawPixel(cursorX + x, cursorY + y, FG_RED, FG_GREEN, FG_BLUE);
        } else {
          DrawPixel(cursorX + x, cursorY + y, BG_RED, BG_GREEN, BG_BLUE);
        }
      }
    }
    cursorX += FONT_WIDTH;
  }
  //if ('A' <= c && c <= 'Z') {
  //  for (unsigned int x = 0; x < FONT_WIDTH; x++) {
  //    for (unsigned int y = 0; y < FONT_HEIGHT; y++) {
  //      if (fontBitMap[c][y][x]) {
  //        DrawPixel(cursorX + x, cursorY + y, FG_RED, FG_GREEN, FG_BLUE);
  //      } else {
  //        DrawPixel(cursorX + x, cursorY + y, BG_RED, BG_GREEN, BG_BLUE);
  //      }
  //    }
  //  }
  //  cursorX += FONT_WIDTH;
  //} else if (c == '\n') {
  //  cursorX = 0;
  //  cursorY += FONT_HEIGHT;
  //}
}

void puts(unsigned char *str) {
  while (*str != '\0') {
    unsigned char c = *str;
    putc(c);
    str++;
  }
}
