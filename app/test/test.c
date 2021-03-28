/**
 * @file test.c
 * @brief アプリ分離のテスト用
 */
#include <syscall.h>
void entry() {
  int pid = Syscall(SYSCALL_FORK, 0, 0, 0);

  char c;
  // 子
  if (pid == 0) {
    c = 'A';
  } else {
    // 親
    c = 'Z';
  }

  while (1) {
    Syscall(SYSCALL_PUT, c, 0, 0);
    volatile unsigned long long wait = 10000000;
    while (wait--)
      ;
  }
}
