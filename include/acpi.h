#ifndef _ACPI_H_
#define _ACPI_H_

struct __attribute__((packed)) RSDP {
  char Signature[8];
  unsigned char Checksum;
  char OEMID[6];
  unsigned char Revision;
  unsigned int RSDTAddress;
  unsigned int Length;
  unsigned long long XSDTAddress;
  unsigned char ExtendedChecksum;
  unsigned char Reserved[3];
};

struct __attribute__((packed)) SDTH {
  char Signature[4];
  unsigned int Length;
  unsigned char Revision;
  unsigned char Checksum;
  char OEMID[6];
  char OEMTableID[8];
  unsigned int OEMRevision;
  unsigned int CreatorID;
  unsigned int CreatorRevision;
};

struct __attribute__((packed)) XSDT {
  struct SDTH Header;
  struct SDTH *Entry[0];
};

struct __attribute__((packed)) ACPI_ADDRESS {
  unsigned char AddressSpaceID;
  unsigned char RegisterBitWidth;
  unsigned char RegisterBitOffset;
  unsigned char AccessSize;
  unsigned long long Address;
};

void ACPIInit(void *RSDTAddress);
struct SDTH *FindSDT(char *findSignature);
void DumpSDTHSignature(struct SDTH *sdth);
void DumpXSDT();

#endif
