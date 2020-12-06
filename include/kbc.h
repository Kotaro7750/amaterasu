#ifndef _KBC_H_
#define _KBC_H_

#define KBC_CONTROLLER_ADDRESS 0x64
#define KBC_ENCODER_ADDRESS 0x60

#define KBC_STATUS_OBF_MASK 0x1
#define KBC_DATA_MAKE_MASK 0x80

#define ASCII_ESC 0x1b
#define ASCII_BS 0x08
#define ASCII_HT 0x09

void KBCInit(void);
char getc(void);
#endif
