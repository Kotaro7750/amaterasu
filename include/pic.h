#ifndef _PIC_H_
#define _PIC_H_

#define PIC_MASTER_COMMAND_STATUS 0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_COMMAND_STATUS 0xa0
#define PIC_SLAVE_DATA 0xa1

#define PIC_MASTER_INTERRUPT_NUM 32
#define PIC_SLAVE_INTERRUPT_NUM 40

#define PIC_EOI 0x60

void picInit(void);
void EnableInterruptOnPIC(unsigned char interruptNum);
void SendEndOfInterrupt(unsigned char interruptNum);
#endif
