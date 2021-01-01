#ifndef _PHYSICALMEMORY_H_
#define _PHYSICALMEMORY_H_

#define PAGE_SIZE 4096

struct PhysicalMemoryFreeMapInfo {
  unsigned long long Base;
  unsigned long long Size;
};

extern struct PhysicalMemoryFreeMapInfo physicalMemoryFreeMapInfo;

void PhysicalMemoryManagementInit(struct PhysicalMemoryFreeMapInfo freeMapInfo);
unsigned long long AllocateSinglePageFrame();
void FreeSinglePageFrame(unsigned long long address);
void FreeSinglePageOnPhysicalMemoryFreeMap(unsigned long long pageIndex);
void SetAllocatedOnPhysicamMemoryFreeMap(unsigned long long pageIndex);

#endif
