.macro SAVE_ALL_REGISTERS
  push %r15
  push %r14
  push %r13
  push %r12
  push %r11
  push %r10
  push %r9
  push %r8
  push %rdi
  push %rsi
  push %rbp
  push %rdx
  push %rcx
  push %rbx
  push %rax
.endm

.macro POP_ALL_REGISTERS_EXCLUDE_RAX
  pop %rbx
  pop %rcx
  pop %rdx
  pop %rbp
  pop %rsi
  pop %rdi
  pop %r8
  pop %r9
  pop %r10
  pop %r11
  pop %r12
  pop %r13
  pop %r14
  pop %r15
.endm

.macro POP_ALL_REGISTERS
  pop %rax
  POP_ALL_REGISTERS_EXCLUDE_RAX
.endm

  .global DefaultHandler
DefaultHandler:
  jmp     DefaultHandler

  .global DoubleFaultHandlerASM
DoubleFaultHandlerASM:
  call    DoubleFaultHandler
  iretq

  .global GeneralProtectionFaultHandlerASM
GeneralProtectionFaultHandlerASM:
  mov %rsp,%rdi
  call    GeneralProtectionFaultHandler
  iretq

  .global PageFaultHandlerASM
PageFaultHandlerASM:
  SAVE_ALL_REGISTERS
  call    PageFaultHandler
  POP_ALL_REGISTERS
  iretq

  .global KBCHandler
KBCHandler:
  SAVE_ALL_REGISTERS
  call    doKBCInterrupt
  POP_ALL_REGISTERS
  iretq

  .global ATAHandlerASM
ATAHandlerASM:
  SAVE_ALL_REGISTERS
  call    ATAHandler
  POP_ALL_REGISTERS
  iretq

  .global HPETHandlerASM
  .global HPETHandlerRet
HPETHandlerASM:
  SAVE_ALL_REGISTERS
  call    HPETHandler
HPETHandlerRet:
  POP_ALL_REGISTERS
  iretq

 .global SyscallHandlerASM
SyscallHandlerASM:
  SAVE_ALL_REGISTERS
  mov %rsp,%rsi
  call    SyscallHandler
  pop %rcx
  POP_ALL_REGISTERS_EXCLUDE_RAX
  iretq

  .global return_from_sys_exec
return_from_sys_exec:
  POP_ALL_REGISTERS
  iretq


  .global return_from_sys_fork
return_from_sys_fork:
  movq $0x20, %rdi
  call SendEndOfInterrupt
  pop %rdi
  POP_ALL_REGISTERS
  iretq
