/**
 * @file process.c
 * @brief プロセスの生成，削除に関連する処理
 */
#include "include/process.h"
#include "include/elf.h"
#include "include/file.h"
#include "include/graphic.h"
#include "include/hpet.h"
#include "include/kHeap.h"
#include "include/paging.h"
#include "include/physicalMemory.h"
#include "include/scheduler.h"
#include "include/util.h"
#include "include/x86_64.h"

/**
 * @brief 新しいプロセスを従来プロセスと同等に扱うために，タイマー割り込みによって中断した時点でのスタックを再現する
 */
unsigned long long SetUpStack(unsigned long long setUpStackBase, unsigned long long processStackBase, struct Registers *registers) {
  unsigned long long *sp = (unsigned long long *)setUpStackBase;

  //----------
  // for IRET
  //----------
  // push SS
  *sp = registers->ds;
  --sp;

  // push old RSP
  *sp = processStackBase;
  --sp;

  // push RFLAGS
  *sp = registers->rflags;
  --sp;

  // push CS
  *sp = registers->cs;
  --sp;

  // push RIP
  *sp = registers->rip;
  --sp;

  *sp = registers->rcx;
  --sp;
  *sp = registers->rax;
  --sp;
  *sp = registers->rdx;
  --sp;
  *sp = registers->rbx;
  --sp;
  *sp = registers->rbp;
  --sp;
  *sp = registers->rsi;
  --sp;
  *sp = registers->rdi;
  --sp;

  //----------
  // HPET handler frame
  //----------
  *sp = (unsigned long long)HPETHandlerRet;
  --sp;

  *sp = processStackBase;
  unsigned long long IRQHandlerFrame = (unsigned long long)sp;
  --sp;

  //----------
  // schedule frame
  //----------
  *sp = (unsigned long long)ScheduleRet;
  --sp;

  *sp = IRQHandlerFrame;
  unsigned long long ScheduleFrame = (unsigned long long)sp;
  --sp;

  *sp = ScheduleFrame;
  --sp;

  // RFLAGS
  *sp = 0x2;

  return (unsigned long long)sp;
}

/**
 * @brief 新しいプロセス用のページテーブルを構築する
 */
unsigned long long SetUpPageTable(unsigned long long textSegmentPageFrame, unsigned long long processStackBase) {
  struct L3PTEntry1GB *l3ptBaseLower = (struct L3PTEntry1GB *)AllocatePageFrames(1);
  for (int i = 0; i < 512; i++) {
    if (i < 5) {
      l3ptBaseLower[i].Present = 1;
      l3ptBaseLower[i].ReadWrite = 1;
      l3ptBaseLower[i].UserSupervisor = 0;
      l3ptBaseLower[i].PageWriteThrough = 0;
      l3ptBaseLower[i].PageCacheDisable = 0;
      l3ptBaseLower[i].Accessed = 0;
      l3ptBaseLower[i].Dirty = 0;
      l3ptBaseLower[i].PageSize = 1;
      l3ptBaseLower[i].Global = 0;
      l3ptBaseLower[i]._ignored = 0;
      l3ptBaseLower[i].PAT = 0;
      l3ptBaseLower[i].Reserved = 0;
      l3ptBaseLower[i].PageFramePhysAddr = ((unsigned long long)((unsigned long long)i * 0x40000000) >> 30);
      l3ptBaseLower[i]._ignored3 = 0;
      l3ptBaseLower[i].ProtectionKey = 0;
      l3ptBaseLower[i].ExecuteDisable = 0;
    } else {
      l3ptBaseLower[i].Present = 0;
    }
  }

  struct L1PTEntry *l1ptBaseHigher = (struct L1PTEntry *)AllocatePageFrames(1);
  for (int i = 0; i < 512; i++) {
    if (i == 510) {
      l1ptBaseHigher[i].Present = 1;
      l1ptBaseHigher[i].ReadWrite = 1;
      l1ptBaseHigher[i].UserSupervisor = 1;
      l1ptBaseHigher[i].PageWriteThrough = 0;
      l1ptBaseHigher[i].PageCacheDisable = 0;
      l1ptBaseHigher[i].Accessed = 0;
      l1ptBaseHigher[i].Dirty = 0;
      l1ptBaseHigher[i].PAT = 0;
      l1ptBaseHigher[i].Global = 0;
      l1ptBaseHigher[i]._ignored = 0;
      l1ptBaseHigher[i].PageFramePhysAddr = (textSegmentPageFrame) >> 12;
      l1ptBaseHigher[i]._ignored2 = 0;
      l1ptBaseHigher[i].ProtectionKey = 0;
      l1ptBaseHigher[i].ExecuteDisable = 0;
    } else if (i == 511) {
      l1ptBaseHigher[i].Present = 1;
      l1ptBaseHigher[i].ReadWrite = 1;
      l1ptBaseHigher[i].UserSupervisor = 1;
      l1ptBaseHigher[i].PageWriteThrough = 0;
      l1ptBaseHigher[i].PageCacheDisable = 0;
      l1ptBaseHigher[i].Accessed = 0;
      l1ptBaseHigher[i].Dirty = 0;
      l1ptBaseHigher[i].PAT = 0;
      l1ptBaseHigher[i].Global = 0;
      l1ptBaseHigher[i]._ignored = 0;
      l1ptBaseHigher[i].PageFramePhysAddr = (processStackBase + 8 - PAGE_SIZE) >> 12;
      l1ptBaseHigher[i]._ignored2 = 0;
      l1ptBaseHigher[i].ProtectionKey = 0;
      l1ptBaseHigher[i].ExecuteDisable = 0;
    } else {
      l1ptBaseHigher[i].Present = 0;
    }
  }

  struct L2PTEntry *l2ptBaseHigher = (struct L2PTEntry *)AllocatePageFrames(1);
  for (int i = 0; i < 512; i++) {
    if (i == 511) {
      l2ptBaseHigher[i].Present = 1;
      l2ptBaseHigher[i].ReadWrite = 1;
      l2ptBaseHigher[i].UserSupervisor = 1;
      l2ptBaseHigher[i].PageWriteThrough = 0;
      l2ptBaseHigher[i].PageCacheDisable = 0;
      l2ptBaseHigher[i].Accessed = 0;
      l2ptBaseHigher[i]._ignored1 = 0;
      l2ptBaseHigher[i].PageSize = 0;
      l2ptBaseHigher[i]._ignored2 = 0;
      l2ptBaseHigher[i].L1PTPhysAddr = ((unsigned long long)(l1ptBaseHigher) >> 12);
      l2ptBaseHigher[i]._ignored3 = 0;
      l2ptBaseHigher[i].ExecuteDisable = 0;
    } else {
      l2ptBaseHigher[i].Present = 0;
    }
  }

  struct L3PTEntry *l3ptBaseHigher = (struct L3PTEntry *)AllocatePageFrames(1);
  for (int i = 0; i < 512; i++) {
    if (i == 511) {
      l3ptBaseHigher[i].Present = 1;
      l3ptBaseHigher[i].ReadWrite = 1;
      l3ptBaseHigher[i].UserSupervisor = 1;
      l3ptBaseHigher[i].PageWriteThrough = 0;
      l3ptBaseHigher[i].PageCacheDisable = 0;
      l3ptBaseHigher[i].Accessed = 0;
      l3ptBaseHigher[i]._ignored1 = 0;
      l3ptBaseHigher[i].PageSize = 0;
      l3ptBaseHigher[i]._ignored2 = 0;
      l3ptBaseHigher[i].L2PTPhysAddr = ((unsigned long long)(l2ptBaseHigher) >> 12);
      l3ptBaseHigher[i]._ignored3 = 0;
      l3ptBaseHigher[i].ExecuteDisable = 0;
    } else {
      l3ptBaseHigher[i].Present = 0;
    }
  }

  struct L4PTEntry *l4ptBase = (struct L4PTEntry *)AllocatePageFrames(1);
  for (int i = 0; i < 512; i++) {
    if (i == 0) {
      l4ptBase[i].Present = 1;
      l4ptBase[i].ReadWrite = 1;
      l4ptBase[i].UserSupervisor = 0;
      l4ptBase[i].PageWriteThrough = 0;
      l4ptBase[i].PageCacheDisable = 0;
      l4ptBase[i].Accessed = 0;
      l4ptBase[i]._ignored1 = 0;
      l4ptBase[i].PageSize = 0;
      l4ptBase[i]._ignored2 = 0;
      l4ptBase[i].L3PTPhysAddr = ((unsigned long long)(l3ptBaseLower) >> 12);
      l4ptBase[i]._ignored3 = 0;
      l4ptBase[i].ExecuteDisable = 0;
    } else if (i == 511) {
      l4ptBase[i].Present = 1;
      l4ptBase[i].ReadWrite = 1;
      l4ptBase[i].UserSupervisor = 1;
      l4ptBase[i].PageWriteThrough = 0;
      l4ptBase[i].PageCacheDisable = 0;
      l4ptBase[i].Accessed = 0;
      l4ptBase[i]._ignored1 = 0;
      l4ptBase[i].PageSize = 0;
      l4ptBase[i]._ignored2 = 0;
      l4ptBase[i].L3PTPhysAddr = ((unsigned long long)(l3ptBaseHigher) >> 12);
      l4ptBase[i]._ignored3 = 0;
      l4ptBase[i].ExecuteDisable = 0;
    } else {
      l4ptBase[i].Present = 0;
    }
  }

  return (unsigned long long)l4ptBase;
}

int execHandler(char *filename) {
  struct File file;
  if (GetFileInfo(filename, &file) == 0) {
    return -1;
  }

  unsigned char *buffer = (unsigned char *)kmalloc(file.size);
  if (buffer == 0x0) {
    return -1;
  }

  if (ReadFile(&file, buffer) != file.size) {
    return -1;
  }

  struct Elf64_Ehdr *ehdr = (struct Elf64_Ehdr *)buffer;

  unsigned long long entryPoint = ehdr->e_entry;
  Elf64_Off e_phoff = ehdr->e_phoff;

  struct Elf64_Phdr *phdr = (struct Elf64_Phdr *)(buffer + e_phoff);

  unsigned long long textSegmentAddress = phdr[0].p_vaddr;
  unsigned long long textSegmentSize = phdr[0].p_memsz;
  Elf64_Off textSegmentOffset = phdr[0].p_offset;

  unsigned long long textSegmentPageFrame = AllocatePageFrames(1);
  memcpy((void *)textSegmentPageFrame, (void *)(buffer + textSegmentOffset), textSegmentSize);

  int newTaskId = NewProcessId();

  // prepare stack
  unsigned long long stackBase = (unsigned long long)(AllocatePageFrames(1) + PAGE_SIZE - 8);
  unsigned long long ring0stackBase = (unsigned long long)(AllocatePageFrames(1) + PAGE_SIZE - 8);
  taskList[newTaskId].ring0stackBase = ring0stackBase;

  struct Registers registers;
  memset(&registers, 0, sizeof(registers));
  registers.cs = CS_SEGMENT_SELECTOR_USER;
  registers.ds = SS_SEGMENT_SELECTOR_USER;
  registers.rflags = 0x202;
  registers.rip = entryPoint;

  taskList[newTaskId].rsp = SetUpStack(ring0stackBase, 0xfffffffffffffff8, &registers);

  taskList[newTaskId].isValid = 1;
  taskList[newTaskId].isStarted = 1;
  taskList[newTaskId].cr3 = SetUpPageTable(textSegmentPageFrame, stackBase);

  return 0;
}

/**
 * @brief exitシステムコールのハンドラ
 * @param[in] status 未使用
 */
void exitHandler(unsigned long long status) {
  puts("\n");
  puth(currentTaskId);
  puts("\n");
  while (1) {
  }
}

/**
 * @brief 新しいプロセスを生成する
 */
void DoFork(char shareVM, struct Registers *registers) {
  int newTaskId = NewProcessId();

  if (shareVM) {
    taskList[newTaskId].cr3 = taskList[currentTaskId].cr3;

    unsigned long long stackBase = (unsigned long long)(AllocatePageFrames(1) + PAGE_SIZE - 8);

    taskList[newTaskId].rsp = SetUpStack(stackBase, stackBase, registers);
    taskList[newTaskId].ring0stackBase = stackBase;
  } else {
  }

  taskList[newTaskId].isValid = 1;
  taskList[newTaskId].isStarted = 1;
}

void KernelThread(unsigned long long fn) {
  struct Registers registers;

  memset(&registers, 0, sizeof(struct Registers));

  registers.rip = fn;
  registers.cs = CS_SEGMENT_SELECTOR_KERNEL;
  registers.rflags = 0x202;

  DoFork(1, &registers);
}
