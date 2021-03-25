#include <hpet.h>
#include <acpi.h>
#include <graphic.h>
#include <interrupt.h>
#include <pic.h>
#include <scheduler.h>

unsigned long long HPETBaseAddress;
unsigned long long HPETCLKPeriod;

void HPETHandlerASM(void);

void HPETInit() {
  struct HPETTable *hpetTable = (struct HPETTable *)FindSDT("HPET");

  HPETBaseAddress = hpetTable->BaseAddress.Address;

  // stop hpet
  union GCR gcr;
  gcr.raw = GCR_VALUE;
  gcr.ENABLE_CNF = 0;
  gcr.LEG_RT_CNF = 1;
  GCR_VALUE = gcr.raw;

  union GCIDR gcidr;
  gcidr.raw = GCIDR_VALUE;
  HPETCLKPeriod = gcidr.COUNTER_CLK_PERIOD;

  union TNCCR tnccr;
  tnccr.raw = TNCCR_VALUE(0);

  tnccr.Tn_INT_TYPE_CNF = 0; // edge trigger
  tnccr.Tn_INT_ENB_CNF = 1;
  tnccr.Tn_TYPE_CNF = 1; // periodic
  tnccr.Tn_VAL_SET_CNF = 0;
  tnccr.Tn_32MODE_CNF = 0;
  tnccr.Tn_FSB_EN_CNF = 0;
  tnccr.Reserved1 = 0;
  tnccr.Reserved2 = 0;
  tnccr.Reserved3 = 0;
  TNCCR_VALUE(0) = tnccr.raw;

  SetInterruptDescriptor(HPET_INTERRUPT_NUM, HPETHandlerASM, 1);
  EnableInterruptOnPIC(HPET_INTERRUPT_NUM);
}

void HPETStartPeriodicTimer(unsigned long long us) {
  union TNCCR tnccr;
  tnccr.raw = TNCCR_VALUE(0);
  tnccr.Tn_VAL_SET_CNF = 1;
  TNCCR_VALUE(0) = tnccr.raw;

  unsigned long long clockCount = us * US_TO_FS / HPETCLKPeriod;
  TNCR_VALUE(0) = clockCount;

  MCR_VALUE = (unsigned long long)0;

  union GCR gcr;
  gcr.raw = GCR_VALUE;
  gcr.ENABLE_CNF = 1;
  GCR_VALUE = gcr.raw;
}

void HPETHandler() {
  Schedule();

  asm volatile (".global ScheduleRet");
  asm volatile ("ScheduleRet:");
  SendEndOfInterrupt(HPET_INTERRUPT_NUM);
}

void DumpGCIDR() {
  union GCIDR gcidr;
  gcidr.raw = *(unsigned long long *)HPETBaseAddress;

  puts("REV ID: ");
  puth(gcidr.REV_ID);
  puts("\r\n");

  puts("NUM TIM CAP: ");
  puth(gcidr.TIMER_NUMBER_CAPACITY);
  puts("\r\n");

  puts("COUNT SIZE CAP: ");
  puth(gcidr.COUNT_SIZE_CAP);
  puts("\r\n");

  puts("LEG RT CAP: ");
  puth(gcidr.LEG_RT_CAP);
  puts("\r\n");

  puts("COUNTER CLK PERIOD: ");
  puth(gcidr.COUNTER_CLK_PERIOD);
  puts("\r\n");
}

void DumpGCR() {
  union GCR gcr;
  gcr.raw = *(unsigned long long *)(HPETBaseAddress + GCR_OFFSET);

  puts("GCR\r\n");

  puts("ENABLE CNF: ");
  puth(gcr.ENABLE_CNF);
  puts("\r\n");

  puts("LEG RT CNF: ");
  puth(gcr.LEG_RT_CNF);
  puts("\r\n");
}

void DumpMCR() {
  unsigned long long mcr = *(unsigned long long *)(HPETBaseAddress + MCR_OFFSET);
  puts("MCR\r\n");

  puth(mcr);
  puts("\r\n");
}
