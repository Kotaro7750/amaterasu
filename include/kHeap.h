/**
 * @file kHeap.h
 * @brief カーネルヒープ処理のヘッダ
 */
#ifndef _K_HEAP_H_
#define _K_HEAP_H_

#define KHEAP_HEAP_SIZE 4096
#define KHEAP_BLOCK_SIZE 256
#define KHEAP_BLOCK_NUM (KHEAP_HEAP_SIZE / KHEAP_BLOCK_SIZE)

#define KHEAP_MAX_ORDER 4

struct kHeapBlock {
  char order;
  char isAllocated;
  unsigned int id;
  struct kHeapBlock *prev;
  struct kHeapBlock *next;
};

void kHeapInit();
unsigned long long kmalloc(unsigned int size);
void kfree(unsigned long long addr);
void AddTokHeapOrder(char order, unsigned int id);
void DeleteFromkHeapOrder(char order, unsigned int id);
void DumpkHeap();
#endif
