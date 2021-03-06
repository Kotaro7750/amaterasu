/**
 * @file interrupt.h
 * @brief 割り込み処理のヘッダ
 */
#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#define MAX_INTR_NUM 256

#define DOUBLE_FAULT_EXCP_NUM 8
#define GENERAL_PROTECTION_FAULT_EXCP_NUM 13
#define PAGE_FAULT_EXCP_NUM 14

/**
 * @brief 割り込みディスクリプタ
 */
struct InterruptDescriptor {
  unsigned short Offset0_15;
  unsigned short SegmentSelector;
  unsigned short IST : 3;
  unsigned short _zero1 : 5;
  unsigned short Type : 4;
  unsigned short _zero2 : 1;
  unsigned short DPL : 2;
  unsigned short P : 1;
  unsigned short Offset16_31;
  unsigned int Offset32_63;
  unsigned int _zeroReserved;
};

void SetInterruptDescriptor(unsigned char interruptNumber, void *handler, unsigned int present);
void idtInit();
#endif
