.macro SAVE_ALL_REGISTERS
  push    %rax
  push    %rcx
  push    %rdx
  push    %rbx
  push    %rbp
  push    %rsi
  push    %rdi
.endm

.macro POP_ALL_REGISTERS
  pop     %rdi
  pop     %rsi
  pop     %rbp
  pop     %rbx
  pop     %rdx
  pop     %rcx
  pop     %rax
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
  call    SyscallHandler
  POP_ALL_REGISTERS
  iretq
