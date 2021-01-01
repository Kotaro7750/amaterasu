#include "include/scheduler.h"
#include "include/graphic.h"
#include "include/hpet.h"
#include "include/interrupt.h"
#include "include/pic.h"
#include "include/syscall.h"

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

  HPETStartPeriodicTimer(SCHEDULER_PERIOD);
}

int NewProcessId(){
  int newProcessId = 0;
  while (taskList[newProcessId].isValid) {
    newProcessId = (newProcessId+1) % MAX_INTR_NUM;
  }

  return newProcessId;
}

// 割り込みが起こった時点でのrsp
void Schedule(unsigned long long currentRsp) {
  taskList[currentTaskId].rsp = currentRsp;

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
