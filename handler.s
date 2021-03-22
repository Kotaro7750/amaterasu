  .global DefaultHandler
DefaultHandler:
  jmp     DefaultHandler

  .global DoubleFaultHandlerASM
DoubleFaultHandlerASM:
  call    DoubleFaultHandler
  iretq

  .global GeneralProtectionFaultHandlerASM
GeneralProtectionFaultHandlerASM:
  call    GeneralProtectionFaultHandler
  iretq

  .global PageFaultHandlerASM
PageFaultHandlerASM:
  push    %rax
  push    %rcx
  push    %rdx
  push    %rbx
  push    %rbp
  push    %rsi
  push    %rdi
  call    PageFaultHandler
  pop     %rdi
  pop     %rsi
  pop     %rbp
  pop     %rbx
  pop     %rdx
  pop     %rcx
  pop     %rax
  iretq

  .global KBCHandler
KBCHandler:
  push    %rax
  push    %rcx
  push    %rdx
  push    %rbx
  push    %rbp
  push    %rsi
  push    %rdi
  call    doKBCInterrupt
  pop     %rdi
  pop     %rsi
  pop     %rbp
  pop     %rbx
  pop     %rdx
  pop     %rcx
  pop     %rax
  iretq

  .global ATAHandlerASM
ATAHandlerASM:
  push    %rax
  push    %rcx
  push    %rdx
  push    %rbx
  push    %rbp
  push    %rsi
  push    %rdi
  call    ATAHandler
  pop     %rdi
  pop     %rsi
  pop     %rbp
  pop     %rbx
  pop     %rdx
  pop     %rcx
  pop     %rax
  iretq

  .global HPETHandlerASM
  .global HPETHandlerRet
HPETHandlerASM:
  push    %rax
  push    %rcx
  push    %rdx
  push    %rbx
  push    %rbp
  push    %rsi
  push    %rdi
  call    HPETHandler
HPETHandlerRet:
  pop     %rdi
  pop     %rsi
  pop     %rbp
  pop     %rbx
  pop     %rdx
  pop     %rcx
  pop     %rax
  iretq

  .global SyscallHandlerASM
SyscallHandlerASM:
  push    %rcx
  push    %rax
  push    %rdx
  push    %rbx
  push    %rbp
  push    %rsi
  push    %rdi
  call    SyscallHandler
  pop     %rdi
  pop     %rsi
  pop     %rbp
  pop     %rbx
  pop     %rdx
  pop     %rcx
  pop     %rcx
  iretq
