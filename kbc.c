#include <kbc.h>
#include <graphic.h>
#include <interrupt.h>
#include <pic.h>
#include <x86_64.h>

const unsigned char scanCode[] = {0x00,     ASCII_ESC, '1', '2', '3', '4', '5', '6', '7', '8',  '9',  '0',  '-',  '=',  ASCII_BS,
                                  ASCII_HT, 'q',       'w', 'e', 'r', 't', 'y', 'u', 'i', 'o',  'p',  '[',  ']',  '\n', 0x00,
                                  'a',      's',       'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',  '\'', '`',  0x00, '\\', 'z',
                                  'x',      'c',       'v', 'b', 'n', 'm', ',', '.', '/', 0x00, 0x00, 0x00, ' '};

unsigned char isShift;

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

char getc(void) { return scanCode[GetKeyCode()]; }

void doKBCInterrupt(void) {
  if (!(InByte(KBC_CONTROLLER_ADDRESS) & KBC_STATUS_OBF_MASK))
    goto kbc_exit;

  unsigned char keyCodeWithMask = InByte(KBC_ENCODER_ADDRESS);
  unsigned char keyCode = keyCodeWithMask & 0x7f;
  unsigned char isBrake = (keyCodeWithMask >> 7) & 1;

  char c = scanCode[keyCode];

  switch (keyCode) {
  case KEYCODE_LSHIFT:
  case KEYCODE_RSHIFT:
    isShift = isBrake == 0 ? 1 : 0;
    break;
  case KEYCODE_LCTRL:
    break;
  case KEYCODE_LALT:
    break;
  default:
    if (isBrake == 0) {
      c = CalcASCIICode(c, isShift);
      if (c == '\n') {
        putc('\r');
        putc('\n');
      } else {
        putc(c);
      }
      break;
    }
  }

kbc_exit:
  SendEndOfInterrupt(KBC_INTERRUPT_NUM);
}

unsigned char CalcASCIICode(unsigned char c, unsigned char isShift) {
  if ('a' <= c && c <= 'z') {
    if (isShift) {
      return c - 'a' + 'A';
    } else {
      return c;
    }
  }

  switch (c) {
  case '1':
    return isShift ? '!' : '1';
    break;
  case '2':
    return isShift ? '@' : '2';
    break;
  case '3':
    return isShift ? '#' : '3';
    break;
  case '4':
    return isShift ? '$' : '4';
    break;
  case '5':
    return isShift ? '%' : '5';
    break;
  case '6':
    return isShift ? '^' : '6';
    break;
  case '7':
    return isShift ? '&' : '7';
    break;
  case '8':
    return isShift ? '*' : '8';
    break;
  case '9':
    return isShift ? '(' : '9';
    break;
  case '0':
    return isShift ? ')' : '0';
    break;
  case '-':
    return isShift ? '_' : '-';
    break;
  case '=':
    return isShift ? '+' : '=';
    break;
  case '[':
    return isShift ? '{' : '[';
    break;
  case ']':
    return isShift ? '}' : ']';
    break;
  case '\n':
    return '\n';
    break;
  case ';':
    return isShift ? ':' : ';';
    break;
  case '\'':
    return isShift ? '\'' : '"';
    break;
  case '`':
    return isShift ? '~' : '`';
    break;
  case '\\':
    return isShift ? '|' : '\\';
    break;
  case ',':
    return isShift ? '<' : ',';
    break;
  case '.':
    return isShift ? '>' : '.';
    break;
  case '/':
    return isShift ? '?' : '/';
    break;
  default:
    return 0;
    break;
  }
}
