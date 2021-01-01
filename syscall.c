#include "include/syscall.h"
#include "include/graphic.h"
#include "include/interrupt.h"
#include "include/pic.h"
#include "include/process.h"

void SyscallHandlerASM(void);

void SyscallInit() {
  void *handler;
  asm volatile("lea SyscallHandlerASM, %[handler]" : [ handler ] "=r"(handler));
  SetInterruptDescriptor(SYSCALL_INTERRUPT_NUM, handler, 1);
  EnableInterruptOnPIC(SYSCALL_INTERRUPT_NUM);
}

unsigned long long SyscallHandler(unsigned long long syscallId,
                                  unsigned long long arg1,
                                  unsigned long long arg2 __attribute__((unused)),
                                  unsigned long long arg3 __attribute__((unused))) {
  unsigned long long ret;

  switch (syscallId) {
  case SYSCALL_PUT:
    putc((char)arg1);
    break;
  case SYSCALL_EXEC:
    execHandler(arg1);
    break;
  case SYSCALL_EXIT:
    exitHandler(arg1);
    break;
  }

  ret = 0xbeefcafe01234567;

  SendEndOfInterrupt(SYSCALL_INTERRUPT_NUM);
  return ret;
}

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
