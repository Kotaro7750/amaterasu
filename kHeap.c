/**
 * @file kHeap.c
 * @brief カーネルヒープ処理
 */
#include <kHeap.h>
#include <graphic.h>
#include <physicalMemory.h>
#include <util.h>

//! カーネルヒープ領域の先頭
unsigned long long kHeapStart;

//! それぞれのブロックの情報の配列
struct kHeapBlock kHeapBlocks[KHEAP_BLOCK_NUM];

//! それぞれのオーダーに属する空きブロックのリスト
struct kHeapBlock *kHeapOrder[KHEAP_MAX_ORDER + 1];

void kHeapInit() {
  kHeapStart = AllocatePageFrames(1);

  for (int i = 0; i < KHEAP_BLOCK_NUM; i++) {
    kHeapBlocks[i].order = 0;
    kHeapBlocks[i].isAllocated = 0;
    kHeapBlocks[i].id = i;
    kHeapBlocks[i].prev = 0x0;
    kHeapBlocks[i].next = 0x0;
  }

  kHeapBlocks[0].order = KHEAP_MAX_ORDER;

  for (char i = 0; i < KHEAP_MAX_ORDER; i++) {
    kHeapOrder[i] = 0x0;
  }
  kHeapOrder[KHEAP_MAX_ORDER] = &(kHeapBlocks[0]);
}

unsigned long long kmalloc(unsigned int size) {
  if (size > KHEAP_HEAP_SIZE) {
    return 0x0;
  }

  unsigned int requestedBlockNumber = size % KHEAP_BLOCK_SIZE == 0 ? size / KHEAP_BLOCK_SIZE : size / KHEAP_BLOCK_SIZE + 1;

  int requestedOrder = log2(requestedBlockNumber);
  if (pow(2, requestedOrder) < requestedBlockNumber) {
    requestedOrder++;
  }

  char currentOrder;
  for (currentOrder = requestedOrder; currentOrder <= KHEAP_MAX_ORDER; currentOrder++) {
    if (kHeapOrder[currentOrder] != 0x0) {
      while (currentOrder > requestedOrder) {
        struct kHeapBlock *front = kHeapOrder[currentOrder];

        DeleteFromkHeapOrder(currentOrder, front->id);

        unsigned int buddyId = front->id + pow(2, currentOrder - 1);
        AddTokHeapOrder(currentOrder - 1, buddyId);
        AddTokHeapOrder(currentOrder - 1, front->id);

        currentOrder--;
      }

      struct kHeapBlock *front = kHeapOrder[currentOrder];
      DeleteFromkHeapOrder(currentOrder, front->id);
      front->order = currentOrder;
      front->isAllocated = 1;

      return kHeapStart + KHEAP_BLOCK_SIZE * front->id;
    }
  }

  return 0x0;
}

void kfree(unsigned long long addr) {
  if (addr < kHeapStart || kHeapStart + KHEAP_HEAP_SIZE <= addr) {
    return;
  }

  unsigned int id = (addr - kHeapStart) / KHEAP_BLOCK_SIZE;
  struct kHeapBlock *freedBlock = &(kHeapBlocks[id]);
  if (freedBlock->isAllocated == 0) {
    return;
  }

  AddTokHeapOrder(freedBlock->order, id);

  for (char order = freedBlock->order; order < KHEAP_MAX_ORDER; order++) {
    unsigned int buddyId = id ^ (1 << order);

    struct kHeapBlock *buddy = &(kHeapBlocks[buddyId]);

    // buddy is allocated
    if (buddy->isAllocated) {
      return;
    }

    DeleteFromkHeapOrder(order, id);
    DeleteFromkHeapOrder(order, buddyId);

    id = id < buddyId ? id : buddyId;
    AddTokHeapOrder(order + 1, id);
  }
}

void AddTokHeapOrder(char order, unsigned int id) {
  struct kHeapBlock *front = kHeapOrder[order];
  struct kHeapBlock *added = &(kHeapBlocks[id]);

  added->order = order;
  added->isAllocated = 0;
  added->prev = 0x0;
  added->next = front;
  kHeapOrder[order] = added;
}

void DeleteFromkHeapOrder(char order, unsigned int id) {
  struct kHeapBlock *block = &(kHeapBlocks[id]);
  struct kHeapBlock *prev = block->prev;
  struct kHeapBlock *next = block->next;

  // re-link
  if (prev == 0x0) {
    kHeapOrder[order] = next;
  } else {
    prev->next = next;
  }

  if (next != 0x0) {
    next->prev = prev;
  }

  // init block
  block->order = 0;
  block->isAllocated = 0;
  block->prev = 0x0;
  block->next = 0x0;
}

void DumpkHeap() {
  for (char order = KHEAP_MAX_ORDER; order >= 0; order--) {
    puth(order);
    puts(" ");
    struct kHeapBlock *block = kHeapOrder[order];
    while (block != 0x0) {
      puth(block->id);
      puts(",");
      block = block->next;
    }
    puts("\n");
  }
}
