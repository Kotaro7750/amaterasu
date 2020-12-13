#include "include/physicalMemory.h"
#include "include/graphic.h"

struct PhysicalMemoryFreeMapInfo physicalMemoryFreeMapInfo;

void PhysicalMemoryManagementInit(struct PhysicalMemoryFreeMapInfo freeMapInfo) {
  physicalMemoryFreeMapInfo.Base = freeMapInfo.Base;
  physicalMemoryFreeMapInfo.Size = freeMapInfo.Size;
}

void FreeSinglePageFrame(unsigned long long address) {
  unsigned long long pageIndex = address >> 12;
  FreeSinglePageOnPhysicalMemoryFreeMap(pageIndex);
}

unsigned long long AllocateSinglePageFrame() {
  unsigned char *base = (unsigned char *)(physicalMemoryFreeMapInfo.Base);
  unsigned long long size = physicalMemoryFreeMapInfo.Size;

  unsigned long long pageIndex;
  for (unsigned long long i = 0; i < size; i++) {
    unsigned char mapByte = base[i];
    if (mapByte != 0) {
      // 一番小さいアドレス(mapByteの中身としては一番高位)を返す
      for (int bitIndex = 1; bitIndex <= 8; bitIndex++) {
        unsigned char shiftedByte = mapByte >> bitIndex;
        if (shiftedByte == 0) {
          pageIndex = 8 * i + (8 - bitIndex);

          SetAllocatedOnPhysicamMemoryFreeMap(pageIndex);

          return pageIndex << 12;
        }
      }
    }
  }

  return 1;
}

// Set pageIndex'th bit of FreeMap to 1
// TODO validatge pageIndex
void FreeSinglePageOnPhysicalMemoryFreeMap(unsigned long long pageIndex) {
  // ex. pageIndex:8 -> byteIndex:1,bitIndex:7
  unsigned long long byteIndex = pageIndex / 8;
  unsigned long long bitIndex = 8 - (pageIndex % 8) - 1;

  unsigned char setByte = 1 << bitIndex;
  ((char *)physicalMemoryFreeMapInfo.Base)[byteIndex] |= setByte;
}

void SetAllocatedOnPhysicamMemoryFreeMap(unsigned long long pageIndex) {
  // ex. pageIndex:8 -> byteIndex:1,bitIndex:7
  unsigned long long byteIndex = pageIndex / 8;
  unsigned long long bitIndex = 8 - (pageIndex % 8) - 1;

  unsigned char inversionByte = 1 << bitIndex;

  // check if free
  if (((unsigned char *)physicalMemoryFreeMapInfo.Base)[byteIndex] & inversionByte) {
    ((unsigned char *)physicalMemoryFreeMapInfo.Base)[byteIndex] ^= inversionByte;
  }
}
