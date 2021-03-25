/**
 * @file scheduler.c
 * @brief タスクのスケジューリング関連の処理
 */
#include <scheduler.h>
#include <graphic.h>
#include <hpet.h>
#include <interrupt.h>
#include <physicalMemory.h>
#include <pic.h>
#include <syscall.h>
#include <x86_64.h>

//! スケジューリング対象のタスク
struct TaskListEntry taskList[TASK_MAX_NUMBER];
//! 現在実行中のタスクのID
int currentTaskId = KERNEL_TASK_ID;

/**
 * @brief スケジューリング処理の初期化
 */
void SchedulerInit() {
  for (int i = 0; i < TASK_MAX_NUMBER; i++) {
    taskList[i].isValid = 0;
    taskList[i].isStarted = 0;
    taskList[i].rsp = 0;
  }

  // kernel task
  taskList[KERNEL_TASK_ID].isValid = 1;
  taskList[KERNEL_TASK_ID].isStarted = 1;
  taskList[KERNEL_TASK_ID].cr3 = GetCR3();
}

/**
 * @brief タイマー割り込みによるスケジューリングを開始
 */
void SchedulerStart() { HPETStartPeriodicTimer(SCHEDULER_PERIOD); }

// TODO infinite loop when no process id is available
/**
 * @brief 新しいタスクのIDを決定する
 */
int NewProcessId() {
  int newProcessId = KERNEL_TASK_ID;
  while (taskList[newProcessId].isValid) {
    newProcessId = (newProcessId + 1) % TASK_MAX_NUMBER;
  }

  return newProcessId;
}

int FindNextTask() {
  int id = currentTaskId;
  while (1) {
    id = (id + 1) % TASK_MAX_NUMBER;
    if (taskList[id].isValid) {
      break;
    }
  }
  return id;
}

/**
 * @brief タスクスイッチを行う
 * @details 現在実行しているタスクのスタックにレジスタの値などを退避し，次のタスクの処理を再開する
 */
void Schedule() {
  struct TaskListEntry *prev = &(taskList[currentTaskId]);

  currentTaskId = FindNextTask();
  struct TaskListEntry *next = &(taskList[currentTaskId]);

  asm volatile("mov %[prev], %%rdi" ::[prev] "D"(prev));
  asm volatile("mov %[next], %%rsi" ::[next] "S"(next) : "%rdi");

  // save RFLAGS, rbp into stack
  asm volatile("push %rbp");
  asm volatile("pushf");

  // save rsp into prev->rsp
  asm volatile("mov %rsp, 0x10(%rdi)");

  // save rip for ret
  asm volatile("movq $.load, 0x20(%rdi)");
  asm volatile("movq $.load, 0x20(%rsi)");

  // --------------------
  // Task Switch
  // --------------------
  // switch page table
  asm volatile("mov %[value], %%cr3" ::[value] "a"(next->cr3));

  // load rsp from next->rsp
  asm volatile("mov 0x10(%rsi), %rsp");

  asm volatile("push %rdi");
  // load rip for ret
  asm volatile("push 0x20(%rsi)");
  asm volatile("mov 0x18(%rsi), %rdi");
  asm volatile("jmp SwitchKernelStack");

  asm volatile(".load:");
  asm volatile("pop %rdi");
  // load RFLAGS, rbp from stack
  asm volatile("popf");
  asm volatile("pop %rbp");
}

void sleep() {
  taskList[currentTaskId].isValid = 0;

  Schedule();

  SendEndOfInterrupt(HPET_INTERRUPT_NUM);
}

void wakeup(int taskId) { taskList[taskId].isValid = 1; }
