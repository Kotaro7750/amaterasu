#ifndef _LIST_H_
#define _LIST_H_

struct List {
  struct List *prev;
  struct List *next;
};

#define LIST_INIT(name)                                                                                                                              \
  {                                                                                                                                                  \
    name.prev = &name;                                                                                                                               \
    name.next = &name;                                                                                                                               \
  }

#define LIST_IS_EMPTY(l) (l->prev == l && l->next == l ? 1 : 0)

// 与えられたList型へのポインタから，そのList型をフィールドとして持つ構造体Tの先頭アドレスを返す
#define LIST_ENTRY(list, T, field) ((T *)(((unsigned long long)list) - (unsigned long long)(&(((T *)(0x0))->field))))

void ListAdd(struct List *added, struct List *prev);
void ListDelete(struct List *deleted);

#endif
