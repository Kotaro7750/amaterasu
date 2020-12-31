#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_
#include "hpet.h"

#define SCHEDULER_PERIOD (100 * MS_TO_US)

void SchedulerInit();
void Schedule(unsigned long long currentRsp);

#endif
