/**
 * @file test.c
 * @brief アプリ分離のテスト用
 */
#include "../lib/include/syscall.h"
void entry() {
  Syscall(SYSCALL_PUT, 'C', 0, 0);
}
