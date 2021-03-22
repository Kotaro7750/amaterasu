#ifndef _HPET_H_
#define _HPET_H_
#include "acpi.h"

#define US_TO_FS 1000000000
#define MS_TO_US 1000
#define SEC_TO_US 1000000

#define GCIDR_OFFSET 0
#define GCIDR_VALUE (*(volatile unsigned long long *)(HPETBaseAddress + GCIDR_OFFSET))

#define GCR_OFFSET 0x10
#define GCR_VALUE (*(volatile unsigned long long *)(HPETBaseAddress + GCR_OFFSET))

#define MCR_OFFSET 0xf0
#define MCR_VALUE (*(volatile unsigned long long *)(HPETBaseAddress + MCR_OFFSET))

#define TIMER_N_OFFSET(n) (0x20 * (n))

#define TNCCR_OFFSET 0x100
#define TNCCR_VALUE(n) (*(volatile unsigned long long *)(HPETBaseAddress + TNCCR_OFFSET + TIMER_N_OFFSET(n)))

#define TNCR_OFFSET 0x108
#define TNCR_VALUE(n) (*(volatile unsigned long long *)(HPETBaseAddress + TNCR_OFFSET + TIMER_N_OFFSET(n)))

#define HPET_INTERRUPT_NUM 32

struct __attribute__((packed)) HPETTable {
  struct SDTH Header;
  unsigned int EventTimerBlockID;
  struct ACPI_ADDRESS BaseAddress;
  unsigned char HPETNumber;
  unsigned short MinimusTick;
  unsigned char flags;
};

union GCIDR {
  unsigned long long raw;
  struct __attribute__((packed)) {
    unsigned long long REV_ID : 8;
    unsigned long long TIMER_NUMBER_CAPACITY : 5; // the amount of timers
    unsigned long long COUNT_SIZE_CAP : 1;        // if 1, 64bit mode is enabled
    unsigned long long Reserved : 1;
    unsigned long long LEG_RT_CAP : 1; // if 1, legacy mode can be used
    unsigned long long VENDOR_ID : 16;
    unsigned long long COUNTER_CLK_PERIOD : 32; // tick period in femtoseconds 10^-15 seconds
  };
};

union GCR {
  unsigned long long raw;
  struct __attribute__((packed)) {
    unsigned long long ENABLE_CNF : 1;
    unsigned long long LEG_RT_CNF : 1;
    unsigned long long Reserved : 62;
  };
};

union TNCCR {
  unsigned long long raw;
  struct __attribute__((packed)) {
    unsigned long long Reserved1 : 1;
    unsigned long long Tn_INT_TYPE_CNF : 1;
    unsigned long long Tn_INT_ENB_CNF : 1;
    unsigned long long Tn_TYPE_CNF : 1;
    unsigned long long Tn_PER_INT_CAP : 1;
    unsigned long long Tn_SIZE_CAP : 1;
    unsigned long long Tn_VAL_SET_CNF : 1;
    unsigned long long Reserved2 : 1;
    unsigned long long Tn_32MODE_CNF : 1;
    unsigned long long Tn_INT_ROUTE_CNF : 5;
    unsigned long long Tn_FSB_EN_CNF : 1;
    unsigned long long Tn_FSB_INT_DEL_CAP : 1;
    unsigned long long Reserved3 : 16;
    unsigned long long Tn_INT_ROUTE_CAP : 32;
  };
};

extern void HPETHandlerRet(void);
extern void ScheduleRet(void);

void HPETInit();
void HPETStartPeriodicTimer(unsigned long long us);
void DumpGCIDR();
void DumpGCR();
void DumpMCR();
#endif
