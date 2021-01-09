#include "include/process.h"
#include "include/graphic.h"
#include "include/paging.h"
#include "include/physicalMemory.h"
#include "include/scheduler.h"

void execHandler(unsigned long long entryPoint) {
  unsigned long long *sp = (unsigned long long *)(AllocateSinglePageFrame() + PAGE_SIZE - 1);
  unsigned long long old_sp = (unsigned long long)sp;

  // enable user mode
  // puth((unsigned long long)sp);

  int newTaskId = NewProcessId();

  /* push SS */
  --sp;
  //*sp = 0x0;
  *sp = 35;

  /* push old RSP */
  --sp;
  *sp = old_sp;

  /* push RFLAGS */
  --sp;
  *sp = 0x202;

  /* push CS */
  --sp;
  //*sp = 8;
  *sp = 27;

  /* push RIP */
  --sp;
  *sp = entryPoint;

  /* push GR */
  unsigned char i;
  for (i = 0; i < 7; i++) {
    --sp;
    *sp = 0;
  }

  struct L3PTEntry1GB *l3ptBase = (struct L3PTEntry1GB *)AllocateSinglePageFrame();
  for (int i = 0; i < 512; i++) {
    if (i < 5) {
      l3ptBase[i].Present = 1;
      l3ptBase[i].ReadWrite = 1;
      l3ptBase[i].UserSupervisor = 1;
      l3ptBase[i].PageWriteThrough = 0;
      l3ptBase[i].PageCacheDisable = 0;
      l3ptBase[i].Accessed = 0;
      l3ptBase[i].Dirty = 0;
      l3ptBase[i].PageSize = 1;
      l3ptBase[i].Global = 0;
      l3ptBase[i]._ignored = 0;
      l3ptBase[i].PAT = 0;
      l3ptBase[i].Reserved = 0;
      l3ptBase[i].PageFramePhysAddr = ((unsigned long long)((unsigned long long)i * 0x40000000) >> 30);
      l3ptBase[i]._ignored3 = 0;
      l3ptBase[i].ProtectionKey = 0;
      l3ptBase[i].ExecuteDisable = 0;
    } else {
      l3ptBase[i].Present = 0;
    }
  }
  struct L4PTEntry *l4ptBase = (struct L4PTEntry *)AllocateSinglePageFrame();
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
      l4ptBase[i].L3PTPhysAddr = ((unsigned long long)(l3ptBase) >> 12);
      l4ptBase[i]._ignored3 = 0;
      l4ptBase[i].ExecuteDisable = 0;
    } else {
      l4ptBase[i].Present = 0;
    }
  }

  taskList[newTaskId].rsp = (unsigned long long)sp;
  taskList[newTaskId].isValid = 1;
  taskList[newTaskId].isStarted = 1;
  taskList[newTaskId].cr3 = (unsigned long long)l4ptBase;
}

void exitHandler(unsigned long long status) {
  puts("\n");
  puth(currentTaskId);
  puts("\n");
  while (1) {
  }
}
