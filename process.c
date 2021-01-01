#include "include/process.h"
#include "include/graphic.h"
#include "include/physicalMemory.h"
#include "include/scheduler.h"

void execHandler(unsigned long long entryPoint) {
  unsigned long long *sp = (unsigned long long *)(AllocateSinglePageFrame() + PAGE_SIZE - 1);
  unsigned long long old_sp = (unsigned long long)sp;

  int newTaskId = NewProcessId();

  /* push SS */
  --sp;
  *sp = 0x0;

  /* push old RSP */
  --sp;
  *sp = old_sp;

  /* push RFLAGS */
  --sp;
  *sp = 0x202;

  /* push CS */
  --sp;
  *sp = 8;

  /* push RIP */
  --sp;
  *sp = entryPoint;

  /* push GR */
  unsigned char i;
  for (i = 0; i < 7; i++) {
    --sp;
    *sp = 0;
  }

  taskList[newTaskId].rsp = (unsigned long long)sp;
  taskList[newTaskId].isValid = 1;
  taskList[newTaskId].isStarted = 1;
}

void exitHandler(unsigned long long status) {
  puts("\n");
  puth(currentTaskId);
  puts("\n");
  while (1) {
  }
}
