/**
 * @file scheduler.c
 * @brief タスクのスケジューリング関連の処理
 */
#include "include/scheduler.h"
#include "include/graphic.h"
#include "include/hpet.h"
#include "include/interrupt.h"
#include "include/physicalMemory.h"
#include "include/pic.h"
#include "include/syscall.h"
#include "include/x86_64.h"

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
}

/**
 * @brief タイマー割り込みによるスケジューリングを開始
 */
void SchedulerStart() { HPETStartPeriodicTimer(SCHEDULER_PERIOD); }

// TODO infinite loop when no process id is available
/**
 * @brief 当たらしいタスクのIDを決定する
 */
int NewProcessId() {
  int newProcessId = KERNEL_TASK_ID;
  while (taskList[newProcessId].isValid) {
    newProcessId = (newProcessId + 1) % TASK_MAX_NUMBER;
  }

  return newProcessId;
}

// rsp of current stack
// note that current stack is always kernel stack
/**
 * @brief タスクスイッチを行う
 * @param[in] currentRsp 呼び出し直前のrsp
 * @details 現在実行しているタスクのスタックにレジスタの値などを退避し，次のタスクの処理を再開する
 */
void Schedule(unsigned long long currentRsp) {
  if (currentTaskId == KERNEL_TASK_ID) {
    taskList[currentTaskId].rsp = currentRsp;
    taskList[currentTaskId].cr3 = GetCR3();
  } else {
    // rsp of process which occurred interrupt
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

  asm volatile("mov %[value], %%cr3" ::[value] "r"(taskList[currentTaskId].cr3));
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
