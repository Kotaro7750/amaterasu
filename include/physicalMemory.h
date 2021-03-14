#ifndef _PHYSICALMEMORY_H_
#define _PHYSICALMEMORY_H_

#define PAGE_SIZE 4096

struct PhysicalMemoryFreeMapInfo {
  unsigned long long Base;
  unsigned long long Size;
};

#define PAGE_ALLOCATED (1 << 0)
#define PAGE_RESERVED (1 << 1)
#define PAGE_TOP (1 << 2)

struct Page {
  unsigned char flags;
  unsigned long long private;
  struct Page *prev;
  struct Page *next;
};

#define PAGE_FRAME_MAX_ORDER 10

extern struct PhysicalMemoryFreeMapInfo physicalMemoryFreeMapInfo;

void PhysicalMemoryManagementInit(struct PhysicalMemoryFreeMapInfo freeMapInfo);
unsigned long long AllocatePageFrames(int requestedPageNumber);
void FreePageFrames(unsigned long long address);
void AddToPageFrameOrder(char order, struct Page *page);
void DeleteFromPageFrameOrder(char order, struct Page *page);

#endif
