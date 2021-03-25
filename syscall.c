/**
 * @file syscall.c
 * @brief システムコール処理
 */
#include "include/syscall.h"
#include "include/ata.h"
#include "include/fat.h"
#include "include/file.h"
#include "include/graphic.h"
#include "include/interrupt.h"
#include "include/paging.h"
#include "include/pic.h"
#include "include/process.h"

void SyscallHandlerASM(void);

void SyscallInit() {
  void *handler;
  asm volatile("lea SyscallHandlerASM, %[handler]" : [ handler ] "=r"(handler));
  SetInterruptDescriptor(SYSCALL_INTERRUPT_NUM, handler, 1);
  EnableInterruptOnPIC(SYSCALL_INTERRUPT_NUM);
}

/**
 * @brief システムコールによるソフトウェア割り込みのハンドラ
 */
unsigned long long
SyscallHandler(unsigned long long syscallId, unsigned long long arg1, unsigned long long arg2, unsigned long long arg3 __attribute__((unused))) {
  unsigned long long ret;

  switch (syscallId) {
  case SYSCALL_READ:
    ret = ReadFile((struct File *)arg1, (unsigned char *)arg2);
    break;
  case SYSCALL_PUT:
    putc((char)arg1);
    break;
  case SYSCALL_OPEN:
    ret = GetFileInfo((char *)arg1, (struct File *)arg2);
    break;
  case SYSCALL_PHYSADDR:
    ret = CalcPhyAddr(arg1);
    break;
  case SYSCALL_EXEC:
    execHandler((char *)arg1);
    break;
  case SYSCALL_EXIT:
    exitHandler(arg1);
    break;
  case SYSCALL_ATA_READ:
    ret = ATARead((unsigned int)arg1, (int)arg2, (unsigned char *)arg3);
    break;
  }

  SendEndOfInterrupt(SYSCALL_INTERRUPT_NUM);
  return ret;
}

/**
 * @brief ユーザープログラムからシステムコールの呼び出しをする
 */
unsigned long long Syscall(unsigned long long syscallId, unsigned long long arg1, unsigned long long arg2, unsigned long long arg3) {
  unsigned long long ret;
  asm volatile("movq %[syscallId], %%rdi\n"
               "movq %[arg1], %%rsi\n"
               "movq %[arg2], %%rdx\n"
               "movq %[arg3], %%rcx\n"
               "int $0x80\n"
               "movq %%rax, %[ret]"
               : [ ret ] "=r"(ret)
               : [ syscallId ] "m"(syscallId), [ arg1 ] "m"(arg1), [ arg2 ] "m"(arg2), [ arg3 ] "m"(arg3));
  return ret;
}
