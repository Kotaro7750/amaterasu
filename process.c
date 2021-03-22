/**
 * @file process.c
 * @brief プロセスの実行関連
 */
#include "include/process.h"
#include "include/graphic.h"
#include "include/hpet.h"
#include "include/paging.h"
#include "include/physicalMemory.h"
#include "include/scheduler.h"
#include "include/x86_64.h"

/**
 * @brief execシステムコールのハンドラ
 * @param[in] entryPoint タスクのエントリポイント
 * @details 4KiBの物理ページを一つ割り当てて，タスク実行中にコンテキストスイッチが起こったかのようにスタックに積む
 * @n タスク用のページテーブル用に物理ページを割り当て，タスク一覧に登録する
 * @see taskList
 */
void execHandler(unsigned long long entryPoint) {
  // unsigned long long stackBase = (unsigned long long)(AllocatePageFrames(1) + PAGE_SIZE - 1);
  unsigned long long stackBase = (unsigned long long)(AllocatePageFrames(1) + PAGE_SIZE - 8);
  // unsigned long long ring0stackBase = (unsigned long long)(AllocatePageFrames(1) + PAGE_SIZE - 1);
  unsigned long long ring0stackBase = (unsigned long long)(AllocatePageFrames(1) + PAGE_SIZE - 8);

  unsigned long long *sp = (unsigned long long *)ring0stackBase;

  int newTaskId = NewProcessId();

  /* push SS */
  *sp = SS_SEGMENT_SELECTOR_USER;
  --sp;

  /* push old RSP */
  *sp = stackBase;
  --sp;

  /* push RFLAGS */
  *sp = 0x202;
  --sp;

  /* push CS */
  *sp = CS_SEGMENT_SELECTOR_USER;
  --sp;

  /* push RIP */
  *sp = entryPoint;
  --sp;

  /* push GR */
  unsigned char i;
  for (i = 0; i < 7; i++) {
    *sp = 0;
    --sp;
  }

  *sp = (unsigned long long)HPETHandlerRet;
  --sp;

  *sp = stackBase;
  unsigned long long IRQHandlerFrame = (unsigned long long)sp;
  --sp;

  *sp = (unsigned long long)ScheduleRet;
  --sp;

  *sp = IRQHandlerFrame;
  unsigned long long ScheduleFrame = (unsigned long long)sp;
  --sp;

  *sp = ScheduleFrame;
  --sp;

  *sp = 0x2;

  struct L3PTEntry1GB *l3ptBaseLower = (struct L3PTEntry1GB *)AllocatePageFrames(1);
  for (int i = 0; i < 512; i++) {
    if (i < 5) {
      l3ptBaseLower[i].Present = 1;
      l3ptBaseLower[i].ReadWrite = 1;
      l3ptBaseLower[i].UserSupervisor = 1;
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
    if (i == 511) {
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
      // l1ptBaseHigher[i].PageFramePhysAddr = (stackBase + 1 - PAGE_SIZE) >> 12;
      l1ptBaseHigher[i].PageFramePhysAddr = (stackBase + 8 - PAGE_SIZE) >> 12;
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
      l4ptBase[i].UserSupervisor = 1;
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

  taskList[newTaskId].rsp = (unsigned long long)sp;
  // taskList[newTaskId].rsp = (unsigned long long)rsp;
  // taskList[newTaskId].rsp = (0xffffffffffffffff - (stackBase - (unsigned long long)sp));

  taskList[newTaskId].ring0stackBase = ring0stackBase;

  taskList[newTaskId].isValid = 1;
  taskList[newTaskId].isStarted = 1;
  taskList[newTaskId].cr3 = (unsigned long long)l4ptBase;
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
