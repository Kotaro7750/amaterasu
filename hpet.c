#include "include/hpet.h"
#include "include/acpi.h"
#include "include/graphic.h"

unsigned long long HPETBaseAddress;

void HPETInit() {
  struct HPETTable *hpetTable = (struct HPETTable *)FindSDT("HPET");

  HPETBaseAddress = hpetTable->BaseAddress.Address;
}

void DumpGCIDR() {
  union GCIDR gcidr;
  gcidr.raw = *(unsigned long long *)HPETBaseAddress;

  puts("REV ID             ");
  puth(gcidr.REV_ID);
  puts("\r\n");

  puts("NUM TIM CAP        ");
  puth(gcidr.TIMER_NUMBER_CAPACITY);
  puts("\r\n");

  puts("COUNT SIZE CAP     ");
  puth(gcidr.COUNT_SIZE_CAP);
  puts("\r\n");

  puts("LEG RT CAP         ");
  puth(gcidr.LEG_RT_CAP);
  puts("\r\n");

  puts("COUNTER CLK PERIOD ");
  puth(gcidr.COUNTER_CLK_PERIOD);
  puts("\r\n");
}
