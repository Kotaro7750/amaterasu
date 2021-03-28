/**
 * @file scheduler.h
 * @brief タスクスケジューリング処理のヘッダ
 */
#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_
#include <hpet.h>
#include <process.h>

#define SCHEDULER_PERIOD (5 * MS_TO_US)
#define TASK_MAX_NUMBER 4
#define PID_INITIAL 0

void SchedulerStart();
int NewProcessId(int pid);
void Schedule();
void sleep();
void wakeup(struct Process *p);

#endif
