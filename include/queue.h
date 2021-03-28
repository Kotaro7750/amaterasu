#ifndef _QUEUE_H_
#define _QUEUE_H_
#include <list.h>

struct Queue {
  struct List guard;
  int size;
};

struct QueueEntry {
  struct List list;
};

#define QUEUE_INIT(name)                                                                                                                             \
  {                                                                                                                                                  \
    name.size = 0;                                                                                                                                   \
    LIST_INIT(name.guard);                                                                                                                           \
  }

#define QUEUE_FRONT(q, T, field)                                                                                                                     \
  (q)->size == 0 ? 0                                                                                                                                 \
                 : ((T *)((unsigned long long)(LIST_ENTRY((q)->guard.next, struct QueueEntry, list)) - (unsigned long long)(&(((T *)0x0)->field))))

void QueuePush(struct Queue *q, struct QueueEntry *pushed);
void QueuePop(struct Queue *q);
#endif
