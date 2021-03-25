/**
 * @file scheduler.h
 * @brief タスクスケジューリング処理のヘッダ
 */
#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_
#include <hpet.h>

#define SCHEDULER_PERIOD (5 * MS_TO_US)
#define TASK_MAX_NUMBER 4
#define KERNEL_TASK_ID 0

/**
 * @struct TaskListEntry
 * @brief それぞれのタスク情報
 */
struct TaskListEntry {
  unsigned long long isValid : 1;   //!< このタスクが有効か
  unsigned long long isStarted : 1; //!< このタスクは既に開始しているか
  unsigned long long Reserved : 62; //!< 予約領域
  unsigned long long cr3;
  unsigned long long rsp;
  unsigned long long ring0stackBase;
  unsigned long long rip;
};

extern struct TaskListEntry taskList[TASK_MAX_NUMBER];
extern int currentTaskId;

void SchedulerInit();
void SchedulerStart();
int NewProcessId();
void Schedule();
void sleep();
void wakeup(int taskId);

#endif
