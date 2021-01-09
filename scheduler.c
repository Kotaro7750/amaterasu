#include "include/scheduler.h"
#include "include/graphic.h"
#include "include/hpet.h"
#include "include/interrupt.h"
#include "include/physicalMemory.h"
#include "include/pic.h"
#include "include/syscall.h"
#include "include/x86_64.h"

struct TaskListEntry taskList[TASK_MAX_NUMBER];
int currentTaskId = KERNEL_TASK_ID;

void SchedulerInit() {
  for (int i = 0; i < TASK_MAX_NUMBER; i++) {
    taskList[i].isValid = 0;
    taskList[i].isStarted = 0;
    taskList[i].rsp = 0;
  }

  // kernel task
  taskList[KERNEL_TASK_ID].isValid = 1;
  taskList[KERNEL_TASK_ID].isStarted = 1;
}

void SchedulerStart() { HPETStartPeriodicTimer(SCHEDULER_PERIOD); }

// TODO infinite loop when no process id is available
int NewProcessId() {
  int newProcessId = KERNEL_TASK_ID;
  while (taskList[newProcessId].isValid) {
    newProcessId = (newProcessId + 1) % TASK_MAX_NUMBER;
  }

  return newProcessId;
}

// 割り込みが起こった時点でのrsp
void Schedule(unsigned long long currentRsp) {
  if (currentTaskId == KERNEL_TASK_ID) {
    taskList[currentTaskId].rsp = currentRsp;
  } else {
    // 割り込みを起こしたタスクのrsp
    unsigned long long previousTaskRsp = *((unsigned long long *)(currentRsp + 8 * 10));
    unsigned long long *sp = (unsigned long long *)previousTaskRsp;

    // ss
    sp--;
    *sp = *((unsigned long long *)(currentRsp + 8 * 11));

    // rsp
    sp--;
    *sp = *((unsigned long long *)(currentRsp + 8 * 10));

    // rflags
    sp--;
    *sp = *((unsigned long long *)(currentRsp + 8 * 9));

    // cs
    sp--;
    *sp = *((unsigned long long *)(currentRsp + 8 * 8));

    // rip
    sp--;
    *sp = *((unsigned long long *)(currentRsp + 8 * 7));

    // general register
    for (int i = 6; i >= 0; i--) {
      sp--;
      *sp = *((unsigned long long *)(currentRsp + 8 * i));
    }

    taskList[currentTaskId].rsp = (unsigned long long)sp;
  }

  // search next task
  while (1) {
    currentTaskId = (currentTaskId + 1) % TASK_MAX_NUMBER;
    if (taskList[currentTaskId].isValid) {
      break;
    }
  }

  SendEndOfInterrupt(HPET_INTERRUPT_NUM);

  asm volatile("mov %[Rsp], %%rsp" ::[Rsp] "a"(taskList[currentTaskId].rsp));
  asm volatile("pop       %rdi");
  asm volatile("pop       %rsi");
  asm volatile("pop       %rbp");
  asm volatile("pop       %rbx");
  asm volatile("pop       %rdx");
  asm volatile("pop       %rcx");
  asm volatile("pop       %rax");
  asm volatile("iretq");
}
