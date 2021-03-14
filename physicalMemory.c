#include "include/physicalMemory.h"
#include "include/graphic.h"
#include "include/util.h"

struct PhysicalMemoryFreeMapInfo physicalMemoryFreeMapInfo;

struct Page *memoryMap;
struct Page *pageFrameOrder[PAGE_FRAME_MAX_ORDER];

void BuddySystemInit() {
  for (char order = 0; order <= PAGE_FRAME_MAX_ORDER; order++) {
    pageFrameOrder[order] = 0x0;
  }

  unsigned long long start = 0;
  unsigned long long totalPageNumber = physicalMemoryFreeMapInfo.Size / sizeof(struct Page);

  while (start < totalPageNumber) {
    while ((memoryMap[start].flags & PAGE_RESERVED) && start < totalPageNumber) {
      start++;
    }
    if (memoryMap[start].flags & PAGE_RESERVED) {
      break;
    }

    // calculate max possible order of this block
    char maxOrder;
    for (char i = 0; i <= PAGE_FRAME_MAX_ORDER; i++) {
      if (start & (1 << i)) {
        maxOrder = i;
        break;
      }

      if (i == PAGE_FRAME_MAX_ORDER) {
        maxOrder = PAGE_FRAME_MAX_ORDER;
      }
    }

    // calculate streak
    unsigned long long maxPageCount = pow(2, maxOrder);
    unsigned long long end = start + 1;
    while (1) {
      if (end == totalPageNumber || (end - start == maxPageCount)) {
        break;
      }
      if (memoryMap[end].flags & PAGE_RESERVED) {
        break;
      }
      end++;
    }

    unsigned long long pageCount = end - start;
    char order = log2(pageCount);

    memoryMap[start].flags |= PAGE_TOP;
    AddToPageFrameOrder(order, &(memoryMap[start]));

    start += pow(2, order);
  }
}

void PhysicalMemoryManagementInit(struct PhysicalMemoryFreeMapInfo freeMapInfo) {
  physicalMemoryFreeMapInfo.Base = freeMapInfo.Base;
  physicalMemoryFreeMapInfo.Size = freeMapInfo.Size;

  memoryMap = (struct Page *)freeMapInfo.Base;

  BuddySystemInit();
}

unsigned long long AllocatePageFrames(int requestedPageNumber) {
  char requestedOrder = log2(requestedPageNumber);
  if (pow(2, requestedOrder) < requestedPageNumber) {
    requestedOrder++;
  }

  if (requestedOrder > PAGE_FRAME_MAX_ORDER) {
    return 0;
  }

  for (char currentOrder = requestedOrder; currentOrder <= PAGE_FRAME_MAX_ORDER; currentOrder++) {
    if (pageFrameOrder[currentOrder] == 0x0) {
      continue;
    }

    while (currentOrder > requestedOrder) {
      struct Page *front = pageFrameOrder[currentOrder];
      DeleteFromPageFrameOrder(currentOrder, front);

      unsigned long long buddyId = (front - memoryMap) ^ (1 << (currentOrder - 1));
      struct Page *buddy = &(memoryMap[buddyId]);

      front->flags |= PAGE_TOP;
      buddy->flags |= PAGE_TOP;

      AddToPageFrameOrder(currentOrder - 1, buddy);
      AddToPageFrameOrder(currentOrder - 1, front);
      currentOrder--;
    }

    struct Page *allocated = pageFrameOrder[currentOrder];
    allocated->flags |= PAGE_ALLOCATED;
    DeleteFromPageFrameOrder(currentOrder, allocated);
    return (allocated - memoryMap) << 12;
  }

  return 0;
}

void FreePageFrames(unsigned long long address) {
  struct Page *freedTopPage = &(memoryMap[address >> 12]);
  char freedOrder = 0;
  while (1) {
    if (memoryMap[(address >> 12) + (1 << freedOrder)].flags & PAGE_TOP) {
      break;
    }
    freedOrder++;
  }

  AddToPageFrameOrder(freedOrder, freedTopPage);
  freedTopPage->flags ^= PAGE_ALLOCATED;

  for (char currentOrder = freedOrder; currentOrder <= PAGE_FRAME_MAX_ORDER; currentOrder++) {
    unsigned long long buddyId = (address >> 12) ^ (1 << currentOrder);
    if (buddyId >= physicalMemoryFreeMapInfo.Size / sizeof(struct Page)) {
      break;
    }

    struct Page *buddy = &(memoryMap[buddyId]);
    if (buddy->private == currentOrder && (buddy->flags & PAGE_TOP) && ((buddy->flags & PAGE_ALLOCATED) == 0) &&
        ((buddy->flags & PAGE_RESERVED) == 0)) {
      DeleteFromPageFrameOrder(currentOrder, freedTopPage);
      DeleteFromPageFrameOrder(currentOrder, buddy);

      AddToPageFrameOrder(currentOrder + 1, freedTopPage);
    } else {
      break;
    }
  }
}

void AddToPageFrameOrder(char order, struct Page *page) {
  struct Page *front = pageFrameOrder[order];
  page->private = order;

  page->prev = 0x0;
  page->next = front;
  if (front != 0x0) {
    front->prev = page;
  }
  pageFrameOrder[order] = page;
}

void DeleteFromPageFrameOrder(char order, struct Page *page) {
  struct Page *prev = page->prev;
  struct Page *next = page->next;

  if (prev == 0x0) {
    pageFrameOrder[order] = next;
  } else {
    prev->next = next;
  }

  if (next != 0x0) {
    next->prev = prev;
  }

  page->prev = 0x0;
  page->next = 0x0;
}
