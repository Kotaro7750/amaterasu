/**
 * @file process.c
 * @brief プロセスの生成，削除に関連する処理
 */
#include <elf.h>
#include <file.h>
#include <graphic.h>
#include <hpet.h>
#include <kHeap.h>
#include <list.h>
#include <paging.h>
#include <physicalMemory.h>
#include <pic.h>
#include <process.h>
#include <scheduler.h>
#include <syscall.h>
#include <util.h>
#include <x86_64.h>

struct Process *currentProcess;

struct List processList;

void return_from_sys_fork(void);

static unsigned long long SetUpKernelStack(struct InterruptStack *registers, unsigned long long kernelStackBase) {
  unsigned long long sp = kernelStackBase;
  sp -= sizeof(struct InterruptStack);
  sp += 8;
  *(struct InterruptStack *)sp = *registers;
  return sp;
}

/**
 * @brief 新しいプロセス用のページテーブルを構築する
 */
static struct L4PTEntry *SetUpPageTable(unsigned long long textSegmentPageFrame, unsigned long long processStackBase) {
  struct L3PTEntry1GB *l3ptBaseLower = (struct L3PTEntry1GB *)AllocatePageFrames(1);
  if (l3ptBaseLower == 0) {
    return 0;
  }
  for (int i = 0; i < 512; i++) {
    if (i < 5) {
      l3ptBaseLower[i].Present = 1;
      l3ptBaseLower[i].ReadWrite = 1;
      l3ptBaseLower[i].UserSupervisor = 0;
      l3ptBaseLower[i].PageWriteThrough = 0;
      l3ptBaseLower[i].PageCacheDisable = 0;
      l3ptBaseLower[i].Accessed = 0;
      l3ptBaseLower[i].Dirty = 0;
      l3ptBaseLower[i].PageSize = 1;
      l3ptBaseLower[i].Global = 0;
      l3ptBaseLower[i]._ignored = 0;
      l3ptBaseLower[i].PAT = 0;
      l3ptBaseLower[i].Reserved = 0;
      l3ptBaseLower[i].PageFramePhysAddr = ((unsigned long long)((unsigned long long)i * 0x40000000) >> 30);
      l3ptBaseLower[i]._ignored3 = 0;
      l3ptBaseLower[i].ProtectionKey = 0;
      l3ptBaseLower[i].ExecuteDisable = 0;
    } else {
      l3ptBaseLower[i].Present = 0;
    }
  }

  struct L1PTEntry *l1ptBaseHigher = (struct L1PTEntry *)AllocatePageFrames(1);
  if (l1ptBaseHigher == 0) {
    return 0;
  }
  for (int i = 0; i < 512; i++) {
    if (i == 510) {
      l1ptBaseHigher[i].Present = 1;
      l1ptBaseHigher[i].ReadWrite = 1;
      l1ptBaseHigher[i].UserSupervisor = 1;
      l1ptBaseHigher[i].PageWriteThrough = 0;
      l1ptBaseHigher[i].PageCacheDisable = 0;
      l1ptBaseHigher[i].Accessed = 0;
      l1ptBaseHigher[i].Dirty = 0;
      l1ptBaseHigher[i].PAT = 0;
      l1ptBaseHigher[i].Global = 0;
      l1ptBaseHigher[i]._ignored = 0;
      l1ptBaseHigher[i].PageFramePhysAddr = (textSegmentPageFrame) >> 12;
      l1ptBaseHigher[i]._ignored2 = 0;
      l1ptBaseHigher[i].ProtectionKey = 0;
      l1ptBaseHigher[i].ExecuteDisable = 0;
    } else if (i == 511) {
      l1ptBaseHigher[i].Present = 1;
      l1ptBaseHigher[i].ReadWrite = 1;
      l1ptBaseHigher[i].UserSupervisor = 1;
      l1ptBaseHigher[i].PageWriteThrough = 0;
      l1ptBaseHigher[i].PageCacheDisable = 0;
      l1ptBaseHigher[i].Accessed = 0;
      l1ptBaseHigher[i].Dirty = 0;
      l1ptBaseHigher[i].PAT = 0;
      l1ptBaseHigher[i].Global = 0;
      l1ptBaseHigher[i]._ignored = 0;
      l1ptBaseHigher[i].PageFramePhysAddr = (processStackBase + 8 - PAGE_SIZE) >> 12;
      l1ptBaseHigher[i]._ignored2 = 0;
      l1ptBaseHigher[i].ProtectionKey = 0;
      l1ptBaseHigher[i].ExecuteDisable = 0;
    } else {
      l1ptBaseHigher[i].Present = 0;
    }
  }

  struct L2PTEntry *l2ptBaseHigher = (struct L2PTEntry *)AllocatePageFrames(1);
  if (l2ptBaseHigher == 0) {
    return 0;
  }
  for (int i = 0; i < 512; i++) {
    if (i == 511) {
      l2ptBaseHigher[i].Present = 1;
      l2ptBaseHigher[i].ReadWrite = 1;
      l2ptBaseHigher[i].UserSupervisor = 1;
      l2ptBaseHigher[i].PageWriteThrough = 0;
      l2ptBaseHigher[i].PageCacheDisable = 0;
      l2ptBaseHigher[i].Accessed = 0;
      l2ptBaseHigher[i]._ignored1 = 0;
      l2ptBaseHigher[i].PageSize = 0;
      l2ptBaseHigher[i]._ignored2 = 0;
      l2ptBaseHigher[i].L1PTPhysAddr = ((unsigned long long)(l1ptBaseHigher) >> 12);
      l2ptBaseHigher[i]._ignored3 = 0;
      l2ptBaseHigher[i].ExecuteDisable = 0;
    } else {
      l2ptBaseHigher[i].Present = 0;
    }
  }

  struct L3PTEntry *l3ptBaseHigher = (struct L3PTEntry *)AllocatePageFrames(1);
  if (l3ptBaseHigher == 0) {
    return 0;
  }
  for (int i = 0; i < 512; i++) {
    if (i == 511) {
      l3ptBaseHigher[i].Present = 1;
      l3ptBaseHigher[i].ReadWrite = 1;
      l3ptBaseHigher[i].UserSupervisor = 1;
      l3ptBaseHigher[i].PageWriteThrough = 0;
      l3ptBaseHigher[i].PageCacheDisable = 0;
      l3ptBaseHigher[i].Accessed = 0;
      l3ptBaseHigher[i]._ignored1 = 0;
      l3ptBaseHigher[i].PageSize = 0;
      l3ptBaseHigher[i]._ignored2 = 0;
      l3ptBaseHigher[i].L2PTPhysAddr = ((unsigned long long)(l2ptBaseHigher) >> 12);
      l3ptBaseHigher[i]._ignored3 = 0;
      l3ptBaseHigher[i].ExecuteDisable = 0;
    } else {
      l3ptBaseHigher[i].Present = 0;
    }
  }

  struct L4PTEntry *l4ptBase = (struct L4PTEntry *)AllocatePageFrames(1);
  if (l4ptBase == 0) {
    return 0;
  }
  for (int i = 0; i < 512; i++) {
    if (i == 0) {
      l4ptBase[i].Present = 1;
      l4ptBase[i].ReadWrite = 1;
      l4ptBase[i].UserSupervisor = 0;
      l4ptBase[i].PageWriteThrough = 0;
      l4ptBase[i].PageCacheDisable = 0;
      l4ptBase[i].Accessed = 0;
      l4ptBase[i]._ignored1 = 0;
      l4ptBase[i].PageSize = 0;
      l4ptBase[i]._ignored2 = 0;
      l4ptBase[i].L3PTPhysAddr = ((unsigned long long)(l3ptBaseLower) >> 12);
      l4ptBase[i]._ignored3 = 0;
      l4ptBase[i].ExecuteDisable = 0;
    } else if (i == 511) {
      l4ptBase[i].Present = 1;
      l4ptBase[i].ReadWrite = 1;
      l4ptBase[i].UserSupervisor = 1;
      l4ptBase[i].PageWriteThrough = 0;
      l4ptBase[i].PageCacheDisable = 0;
      l4ptBase[i].Accessed = 0;
      l4ptBase[i]._ignored1 = 0;
      l4ptBase[i].PageSize = 0;
      l4ptBase[i]._ignored2 = 0;
      l4ptBase[i].L3PTPhysAddr = ((unsigned long long)(l3ptBaseHigher) >> 12);
      l4ptBase[i]._ignored3 = 0;
      l4ptBase[i].ExecuteDisable = 0;
    } else {
      l4ptBase[i].Present = 0;
    }
  }

  return l4ptBase;
}

static struct ProcessMemory *CopyProcessMemory(char cloneFlags, struct ProcessMemory *originalPm) {
  struct ProcessMemory *pm = (struct ProcessMemory *)kmalloc(sizeof(struct ProcessMemory));
  memcpy(pm, originalPm, sizeof(struct ProcessMemory));

  pm->userStackPageFrame = AllocatePageFrames(1);
  pm->kernelStackPageFrame = AllocatePageFrames(1);

  if (cloneFlags & CLONE_VM) {
    pm->textSegmentPageFrame = originalPm->textSegmentPageFrame;
  } else {
    pm->textSegmentPageFrame = AllocatePageFrames(1);

    memcpy((void *)pm->textSegmentPageFrame, (void *)originalPm->textSegmentPageFrame, PAGE_SIZE);
    memcpy((void *)pm->kernelStackPageFrame, (void *)originalPm->kernelStackPageFrame, PAGE_SIZE);
    memcpy((void *)pm->userStackPageFrame, (void *)originalPm->userStackPageFrame, PAGE_SIZE);
  }
  pm->l4PageTableBase = SetUpPageTable(pm->textSegmentPageFrame, pm->userStackPageFrame - 8 + PAGE_SIZE);

  return pm;
}

static struct ThreadInfo *CopyThreadInfo(char cloneFlags, struct ThreadInfo *originalTi, struct Process *process) {
  struct ThreadInfo *ti = (struct ThreadInfo *)kmalloc(sizeof(struct ThreadInfo));
  memcpy(ti, originalTi, sizeof(struct ThreadInfo));
  ti->process = process;

  return ti;
}

static struct Process *CopyProcess(char cloneFlags, struct Process *original) {
  struct Process *p = (struct Process *)kmalloc(sizeof(struct Process));
  memcpy(p, original, sizeof(struct Process));

  ListAdd(&(p->processes), &(original->processes));

  p->pid = NewProcessId(original->pid);

  p->processMemory = CopyProcessMemory(cloneFlags, original->processMemory);

  p->threadInfo = CopyThreadInfo(cloneFlags, original->threadInfo, p);

  return p;
}

int sysExec(unsigned long long rsp, char *filename) {
  struct File file;
  if (GetFileInfo(filename, &file, 1) == 0) {
    return -1;
  }

  unsigned char *buffer = (unsigned char *)kmalloc(file.size);
  if (buffer == 0x0) {
    return -1;
  }

  if (ReadFile(&file, buffer, 1) != file.size) {
    return -1;
  }

  // parse ELF header
  struct Elf64_Ehdr *ehdr = (struct Elf64_Ehdr *)buffer;
  unsigned long long entryPoint = ehdr->e_entry;
  struct Elf64_Phdr *phdr = (struct Elf64_Phdr *)(buffer + ehdr->e_phoff);

  unsigned long long textSegmentSize = phdr[0].p_memsz;
  Elf64_Off textSegmentOffset = phdr[0].p_offset;

  // copy .text segment to process's own text segment
  struct ProcessMemory *processMemory = currentProcess->processMemory;
  memcpy((void *)processMemory->textSegmentPageFrame, (void *)(buffer + textSegmentOffset), textSegmentSize);

  // iretで戻るための情報を書き換える．rspは初期化，レジスタは引数を除き初期化,ripはエントリポイント,CS,SSは必ずユーザー
  struct InterruptStack registers;
  memset(&registers, 0, sizeof(struct InterruptStack));
  registers.rip = entryPoint;
  registers.rsp = 0xfffffffffffffff8;
  registers.rbp = registers.rsp;
  registers.rflags = 0x202;
  registers.cs = CS_SEGMENT_SELECTOR_USER;
  registers.ss = SS_SEGMENT_SELECTOR_USER;

  unsigned long long *sp = (unsigned long long *)rsp;
  // TODO 汚い
  *sp = registers.rax;
  sp++;
  *sp = registers.rbx;
  sp++;
  *sp = registers.rcx;
  sp++;
  *sp = registers.rdx;
  sp++;
  *sp = registers.rbp;
  sp++;
  *sp = registers.rsi;
  sp++;
  *sp = registers.rdi;
  sp++;
  *sp = registers.r8;
  sp++;
  *sp = registers.r9;
  sp++;
  *sp = registers.r10;
  sp++;
  *sp = registers.r11;
  sp++;
  *sp = registers.r12;
  sp++;
  *sp = registers.r13;
  sp++;
  *sp = registers.r14;
  sp++;
  *sp = registers.r15;
  sp++;
  *sp = registers.rip;
  sp++;
  *sp = registers.cs;
  sp++;
  *sp = registers.rflags;
  sp++;
  *sp = registers.rsp;
  sp++;
  *sp = registers.ss;

  SendEndOfInterrupt(SYSCALL_INTERRUPT_NUM);
  SwitchKernelStack(processMemory->kernelStackPageFrame - 8 + PAGE_SIZE);

  kfree((unsigned long long)buffer);

  asm volatile("mov %[rsp], %%rsp" ::[rsp] "r"(rsp));
  asm volatile("jmp return_from_sys_exec");

  return 0;
}

void sysWaitPid(){
}

/**
 * @brief exitシステムコールのハンドラ
 * @param[in] status 未使用
 */
void sysExit() {
  currentProcess->state = PROCESS_ZOMBIE;
  Schedule();
}

/**
 * @brief 新しいプロセスを生成する
 */
int DoFork(char shareVM, struct InterruptStack *registers, unsigned long long rsp) {
  struct Process *child;
  struct Process *parent = currentProcess;
  child = CopyProcess(shareVM, currentProcess);
  child->parent = parent;

  if (shareVM) {
    registers->rbp = child->processMemory->kernelStackPageFrame - 8 + PAGE_SIZE;
    registers->rsp = child->processMemory->kernelStackPageFrame - 8 + PAGE_SIZE;

    child->threadInfo->ring0rsp = SetUpKernelStack(registers, child->processMemory->kernelStackPageFrame - 8 + PAGE_SIZE);
  } else {
    // returns 0 to child process
    unsigned long long childRsp = child->processMemory->kernelStackPageFrame + (rsp - parent->processMemory->kernelStackPageFrame);
    ((struct InterruptStack *)childRsp)->rax = 0;

    child->threadInfo->ring0rsp = childRsp;
  }

  child->threadInfo->rsp = child->threadInfo->ring0rsp;
  child->threadInfo->rip = (unsigned long long)return_from_sys_fork;
  child->threadInfo->cr3 = (unsigned long long)child->processMemory->l4PageTableBase;

  return child->pid;
}

int sysFork(unsigned long long rsp) { return DoFork(0, (struct InterruptStack *)rsp, rsp); }

void KernelThread(unsigned long long fn, unsigned long long arg1) {
  struct InterruptStack registers;

  memset(&registers, 0, sizeof(struct InterruptStack));

  registers.rip = fn;
  registers.cs = CS_SEGMENT_SELECTOR_KERNEL;
  registers.rflags = 0x202;

  registers.rdi = arg1;

  DoFork(1, &registers, 0x0);
}

void ProcessInit() {
  LIST_INIT(processList);

  currentProcess = (struct Process *)kmalloc(sizeof(struct Process));
  ListAdd(&(currentProcess->processes), &processList);

  currentProcess->pid = PID_INITIAL;
  currentProcess->parent = 0x0;
  currentProcess->state = PROCESS_RUNNING;

  // Allocate process own memory
  struct ProcessMemory *processMemory = (struct ProcessMemory *)kmalloc(sizeof(struct ProcessMemory));

  processMemory->textSegmentPageFrame = AllocatePageFrames(1);
  processMemory->kernelStackPageFrame = AllocatePageFrames(1);
  processMemory->userStackPageFrame = AllocatePageFrames(1);
  processMemory->l4PageTableBase = SetUpPageTable(processMemory->textSegmentPageFrame, processMemory->userStackPageFrame - 8 + PAGE_SIZE);

  SwitchKernelStack(processMemory->kernelStackPageFrame - 8 + PAGE_SIZE);

  // Set up thread infomation
  struct ThreadInfo *threadInfo = (struct ThreadInfo *)kmalloc(sizeof(struct ThreadInfo));

  threadInfo->process = currentProcess;
  threadInfo->ring0rsp = processMemory->kernelStackPageFrame - 8 + PAGE_SIZE;

  currentProcess->processMemory = processMemory;
  currentProcess->threadInfo = threadInfo;
}
