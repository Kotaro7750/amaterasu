#ifndef _KBC_H_
#define _KBC_H_

#define KBC_CONTROLLER_ADDRESS 0x64
#define KBC_ENCODER_ADDRESS 0x60

#define KBC_STATUS_OBF_MASK 0x1
#define KBC_DATA_MAKE_MASK 0x80

#define KBC_INTERRUPT_NUM 33

#define ASCII_ESC 0x1b
#define ASCII_BS 0x08
#define ASCII_HT 0x09

#define KEYCODE_LSHIFT 0x2a
#define KEYCODE_RSHIFT 0x36
#define KEYCODE_LCTRL 0x1d
#define KEYCODE_LALT 0x38

void KBCInit(void);
char getc(void);
unsigned char CalcASCIICode(unsigned char c, unsigned char isShift);
#endif
