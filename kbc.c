#include "include/kbc.h"
#include "include/graphic.h"
#include "include/interrupt.h"
#include "include/pic.h"
#include "include/x86_64.h"

const char keyMap[] = {0x00, ASCII_ESC, '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '^',  ASCII_BS, ASCII_HT, 'q',  'w',  'e',
                       'r',  't',       'y',  'u',  'i',  'o',  'p',  '@',  '[',  '\n', 0x00, 'a',  's',  'd',  'f',      'g',      'h',  'j',  'k',
                       'l',  ';',       ':',  0x00, 0x00, ']',  'z',  'x',  'c',  'v',  'b',  'n',  'm',  ',',  '.',      '/',      0x00, '*',  0x00,
                       ' ',  0x00,      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '7',      '8',      '9',  '-',  '4',
                       '5',  '6',       '+',  '1',  '2',  '3',  '0',  '.',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     0x00,     0x00, 0x00, 0x00,
                       0x00, 0x00,      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     0x00,     0x00, 0x00, 0x00,
                       0x00, '_',       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\\', 0x00, 0x00};

void KBCHandler(void);

void KBCInit(void) {
  void *handler;
  asm volatile("lea KBCHandler, %[handler]" : [ handler ] "=r"(handler));
  SetInterruptDescriptor(KBC_INTERRUPT_NUM, handler, 1);
  EnableInterruptOnPIC(KBC_INTERRUPT_NUM);
}

unsigned char GetKBCData(void) {
  // wait until Output Buffer Full
  while (!(InByte(KBC_CONTROLLER_ADDRESS) & KBC_STATUS_OBF_MASK)) {
  }

  return InByte(KBC_ENCODER_ADDRESS);
}

unsigned char GetKeyCode(void) {
  unsigned char keyData;
  while (((keyData = GetKBCData()) & KBC_DATA_MAKE_MASK)) {
  }

  // since make bit is 0, keyData is keyCode itself
  return keyData;
}

char getc(void) { return keyMap[GetKeyCode()]; }

void doKBCInterrupt(void) {
  if (!(InByte(KBC_CONTROLLER_ADDRESS) & KBC_STATUS_OBF_MASK))
    goto kbc_exit;

  unsigned char keyCode = InByte(KBC_ENCODER_ADDRESS);
  if (keyCode & KBC_DATA_MAKE_MASK)
    goto kbc_exit;

  char c = keyMap[keyCode];
  if (('a' <= c) && (c <= 'z'))
    c = c - 'a' + 'A';
  else if (c == '\n')
    putc('\r');
  putc(c);

kbc_exit:
  SendEndOfInterrupt(KBC_INTERRUPT_NUM);
}
