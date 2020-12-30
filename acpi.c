#include "include/acpi.h"
#include "include/graphic.h"
#include "include/util.h"

struct XSDT *xsdt;

void ACPIInit(void *RSDTAddress) { xsdt = (struct XSDT *)(((struct RSDP *)RSDTAddress)->XSDTAddress); }

struct SDTH *FindSDT(char *findSignature) {
  unsigned int length = xsdt->Header.Length;

  unsigned long long entryNum = (length - sizeof(struct SDTH)) / sizeof(struct SDTH *);

  struct SDTH *sdt;
  for (unsigned long long i = 0; i < entryNum; i++) {
    sdt = xsdt->Entry[i];
    char *signature = sdt->Signature;

    if (!strncmp(signature, findSignature, 4)) {
      return sdt;
    }
  }

  return 0;
}

void DumpSDTHSignature(struct SDTH *sdth) {
  unsigned char i;
  for (i = 0; i < 4; i++)
    putc(sdth->Signature[i]);
}

void DumpXSDT() {
  unsigned int length = xsdt->Header.Length;

  unsigned long long entryNum = (length - sizeof(struct SDTH)) / sizeof(struct SDTH *);

  struct SDTH *entryBase;
  for (unsigned long long i = 0; i < entryNum; i++) {
    entryBase = xsdt->Entry[i];
    DumpSDTHSignature(entryBase);
    puts("\n");
  }
}
