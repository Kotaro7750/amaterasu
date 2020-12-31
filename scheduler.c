#include "include/scheduler.h"
#include "include/graphic.h"
#include "include/hpet.h"
#include "include/pic.h"

unsigned long long taskRsp[2];
int currentTaskId = 0;

unsigned char taskB_stack[4096];

void taskB() {
  while (1) {
    putc('B');
    volatile unsigned long long wait = 10000000;
    while (wait--)
      ;
  }
}

void SchedulerInit() {
  unsigned long long *sp = (unsigned long long *)((unsigned char *)taskB_stack + 4096);
  unsigned long long old_sp = (unsigned long long)sp;

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
  *sp = (unsigned long long)taskB;

  /* push GR */
  unsigned char i;
  for (i = 0; i < 7; i++) {
    --sp;
    *sp = 0;
  }

  taskRsp[1] = (unsigned long long)sp;

  HPETStartPeriodicTimer(SCHEDULER_PERIOD);
}

// 割り込みが起こった時点でのrsp
void Schedule(unsigned long long currentRsp) {
  //int wait = 1;
  //while (wait) {
  //}
  taskRsp[currentTaskId] = currentRsp;
  currentTaskId = (currentTaskId + 1) % 2;
  SendEndOfInterrupt(HPET_INTERRUPT_NUM);

  asm volatile("mov %[Rsp], %%rsp" ::[Rsp] "a"(taskRsp[currentTaskId]));
  asm volatile("pop       %rdi");
  asm volatile("pop       %rsi");
  asm volatile("pop       %rbp");
  asm volatile("pop       %rbx");
  asm volatile("pop       %rdx");
  asm volatile("pop       %rcx");
  asm volatile("pop       %rax");
  asm volatile("iretq");
}
