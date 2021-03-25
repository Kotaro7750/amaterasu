#include <graphic.h>
#include <fb.h>
#include <font.h>

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
}

void puts(unsigned char *str) {
  while (*str != '\0') {
    unsigned char c = *str;
    putc(c);
    str++;
  }
}

void puth(unsigned long long num) {
  int i = 0;
  unsigned char str[UNUM64_STR_LEN + 1];
  for (i = 0; i < UNUM64_STR_LEN; i++) {
    str[i] = '0';
  }
  str[UNUM64_STR_LEN] = '\0';

  i = UNUM64_STR_LEN - 1;
  while (num != 0) {
    unsigned char remainder = num & 0xf;
    num = num >> 4;
    if (remainder >= 10) {
      str[i] = 'A' + remainder - 10;
    } else {
      str[i] = '0' + remainder;
    }
    i--;
  }

  puts(str);
}
