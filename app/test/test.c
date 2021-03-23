/**
 * @file test.c
 * @brief アプリ分離のテスト用
 */
#include "../lib/include/syscall.h"
void entry() {
  while (1) {
    Syscall(SYSCALL_PUT, 'Z', 0, 0);
    volatile unsigned long long wait = 10000000;
    while (wait--)
      ;
  }
}
