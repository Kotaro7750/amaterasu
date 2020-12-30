#ifndef _HPET_H_
#define _HPET_H_
#include "acpi.h"

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

void DumpGCIDR();
#endif
