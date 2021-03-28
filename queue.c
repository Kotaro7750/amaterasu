#include <list.h>
#include <queue.h>

void QueuePush(struct Queue *q, struct QueueEntry *pushed) {
  ListAdd(&pushed->list, q->guard.prev);

  q->size++;
}

void QueuePop(struct Queue *q) {
  if (q->size == 0) {
    return;
  }

  ListDelete(q->guard.next);
  q->size--;
}
