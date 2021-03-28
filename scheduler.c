/**
 * @file scheduler.c
 * @brief タスクのスケジューリング関連の処理
 */
#include <graphic.h>
#include <hpet.h>
#include <interrupt.h>
#include <list.h>
#include <physicalMemory.h>
#include <pic.h>
#include <process.h>
#include <scheduler.h>
#include <syscall.h>
#include <x86_64.h>

/**
 * @brief タイマー割り込みによるスケジューリングを開始
 */
void SchedulerStart() { HPETStartPeriodicTimer(SCHEDULER_PERIOD); }

// TODO pid pool, collision detection
/**
 * @brief 新しいタスクのIDを決定する
 */
int NewProcessId(int pid) { return pid + 1; }

struct Process *FindNextTask(struct Process *p) {
  struct List *pl = &p->processes;
  while (1) {
    pl = pl->next;
    if (pl != &processList && LIST_ENTRY(pl, struct Process, processes)->state == PROCESS_RUNNING) {
      break;
    }
  }

  return LIST_ENTRY(pl, struct Process, processes);
}

/**
 * @brief タスクスイッチを行う
 * @details 現在実行しているタスクのスタックにレジスタの値などを退避し，次のタスクの処理を再開する
 */
void Schedule() {
  struct Process *prev = currentProcess;
  struct Process *next = FindNextTask(prev);

  if (prev == next) {
    return;
  }
  currentProcess = next;

  asm volatile("mov %[prev], %%rdi" ::[prev] "D"(prev->threadInfo));
  asm volatile("mov %[next], %%rsi" ::[next] "S"(next->threadInfo) : "%rdi");

  // save RFLAGS, rbp into stack
  asm volatile("push %rbp");
  asm volatile("pushf");

  // save rsp into prev->threadInfo->registers.rsp
  asm volatile("mov %rsp, 0x90(%rdi)");

  // save rip for ret
  asm volatile("movq $.load, 0x78(%rdi)");

  // --------------------
  // Task Switch
  // --------------------
  // switch page table
  asm volatile("mov %[value], %%cr3" ::[value] "a"(next->processMemory->l4PageTableBase));

  // load rsp from next->threadInfo->rsp
  asm volatile("mov 0x90(%rsi), %rsp");

  // save rdi
  asm volatile("push %rdi");

  // load rip for ret
  asm volatile("push 0x78(%rsi)");

  // switch kernel stack to next->threadInfo->registers.ring0rsp
  asm volatile("mov 0xb0(%rsi), %rdi");
  asm volatile("jmp SwitchKernelStack");

  asm volatile(".load:");
  asm volatile("pop %rdi");

  // load RFLAGS, rbp from stack
  asm volatile("popf");
  asm volatile("pop %rbp");
}

void sleep() {
  currentProcess->state = PROCESS_SLEEP;

  Schedule();

  SendEndOfInterrupt(HPET_INTERRUPT_NUM);
}

void wakeup(struct Process *p) { p->state = PROCESS_RUNNING; }
