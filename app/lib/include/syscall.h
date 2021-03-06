#ifndef _SYSCALL_H_
#define _SYSCALL_H_
enum SYSCALL_NUM {
  SYSCALL_READ = 0,
  SYSCALL_PUT = 1,
  SYSCALL_OPEN = 2,
  SYSCALL_PHYSADDR = 3,
  SYSCALL_EXEC = 59,
  SYSCALL_EXIT = 60,
  SYSCALL_ATA_READ = 61,
  SYSCALL_FORK = 62,
};

unsigned long long Syscall(unsigned long long syscallId, unsigned long long arg1, unsigned long long arg2, unsigned long long arg3);
#endif
