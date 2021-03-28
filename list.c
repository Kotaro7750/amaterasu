/**
 * @brief 汎用的な双方向リスト関連の処理
 */

#include <list.h>

/**
 * @brief ある要素の直後に挿入する
 */
void ListAdd(struct List *added, struct List *prev) {
  struct List *next = prev->next;

  prev->next = added;
  added->prev = prev;

  next->prev = added;
  added->next = next;
}

void ListDelete(struct List *deleted) {
  struct List *prev = deleted->prev;
  struct List *next = deleted->next;

  prev->next = next;
  next->prev = prev;
}
