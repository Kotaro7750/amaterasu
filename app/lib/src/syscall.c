unsigned long long Syscall(unsigned long long syscallId, unsigned long long arg1, unsigned long long arg2, unsigned long long arg3) {
  unsigned long long ret;
  asm volatile("movq %[syscallId], %%rdi\n"
               "movq %[arg1], %%rdx\n"
               "movq %[arg2], %%rcx\n"
               "movq %[arg3], %%r8\n"
               "int $0x80\n"
               "movq %%rax, %[ret]"
               : [ ret ] "=r"(ret)
               : [ syscallId ] "m"(syscallId), [ arg1 ] "m"(arg1), [ arg2 ] "m"(arg2), [ arg3 ] "m"(arg3));
  return ret;
}
