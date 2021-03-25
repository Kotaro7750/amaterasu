#include <pic.h>
#include <x86_64.h>

void picInit(void) {
  // ICW1, use ICW4, enable cascade, edge trigger
  OutByte(PIC_MASTER_COMMAND_STATUS, 0x11);
  OutByte(PIC_SLAVE_COMMAND_STATUS, 0x11);

  // ICW2
  OutByte(PIC_MASTER_DATA, PIC_MASTER_INTERRUPT_NUM);
  OutByte(PIC_SLAVE_DATA, PIC_SLAVE_INTERRUPT_NUM);

  // ICW3, slave is connected to IR2
  OutByte(PIC_MASTER_DATA, 0x4);
  OutByte(PIC_SLAVE_DATA, 0x2);

  // ICW4, x86
  OutByte(PIC_MASTER_DATA, 0x1);
  OutByte(PIC_SLAVE_DATA, 0x1);

  // OCW1, mask all IRQ
  OutByte(PIC_MASTER_DATA, 0xff);
  OutByte(PIC_SLAVE_DATA, 0xff);
}

void EnableInterruptOnPIC(unsigned char interruptNum) {
  unsigned char interruptOffset;

  if (PIC_MASTER_INTERRUPT_NUM <= interruptNum && interruptNum < PIC_SLAVE_INTERRUPT_NUM) {
    interruptOffset = interruptNum - PIC_MASTER_INTERRUPT_NUM;

  } else if (PIC_SLAVE_INTERRUPT_NUM <= interruptNum && interruptNum < PIC_SLAVE_INTERRUPT_NUM + 8) {
    interruptOffset = interruptNum - PIC_SLAVE_INTERRUPT_NUM;
    unsigned char interruptBit = 1 << interruptOffset;

    unsigned char IMR = InByte(PIC_SLAVE_DATA);

    if (IMR & interruptBit) {
      OutByte(PIC_SLAVE_DATA, IMR - interruptBit);
    }

    // for master
    interruptOffset = 2;

  } else {
    return;
  }

  unsigned char interruptBit = 1 << interruptOffset;

  unsigned char IMR = InByte(PIC_MASTER_DATA);

  // when already enabled
  if (!(IMR & interruptBit)) {
    return;
  }

  OutByte(PIC_MASTER_DATA, IMR - interruptBit);
}

void SendEndOfInterrupt(unsigned char interruptNum) {
  if (PIC_MASTER_INTERRUPT_NUM <= interruptNum && interruptNum < PIC_SLAVE_INTERRUPT_NUM) {
    unsigned char irq = interruptNum - PIC_MASTER_INTERRUPT_NUM;

    OutByte(PIC_MASTER_COMMAND_STATUS, PIC_EOI | irq);
  } else if (PIC_SLAVE_INTERRUPT_NUM <= interruptNum && interruptNum < PIC_SLAVE_INTERRUPT_NUM + 8) {
    unsigned char irq = interruptNum - PIC_SLAVE_INTERRUPT_NUM;

    OutByte(PIC_MASTER_COMMAND_STATUS, PIC_EOI | 0x2);
    OutByte(PIC_SLAVE_COMMAND_STATUS, PIC_EOI | irq);
  } else {
  }
}
