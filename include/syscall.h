/**
 * @file syscall.h
 * @brief システムコール処理のヘッダ
 */
#ifndef _SYSCALL_H_
#define _SYSCALL_H_
#define SYSCALL_INTERRUPT_NUM 0x80

enum SYSCALL_NUM {
  SYSCALL_READ = 0,
  SYSCALL_PUT = 1,
  SYSCALL_OPEN = 2,
  SYSCALL_PHYSADDR = 3,
  SYSCALL_EXEC = 59,
  SYSCALL_EXIT = 60,
  SYSCALL_ATA_READ = 61
};

void SyscallInit();
unsigned long long Syscall(unsigned long long syscallId, unsigned long long arg1, unsigned long long arg2, unsigned long long arg3);
#endif
