/**
 * @file process.h
 * @brief タスク関連のヘッダ
 */
#ifndef _PROCESS_H_
#define _PROCESS_H_

/*
 * clone flags
 */
#define CLONE_VM 0x00000001 // set if VM shared between processes

#include <list.h>
#include <paging.h>
#include <x86_64.h>

enum ProcessState {
  PROCESS_RUNNING,
  PROCESS_SLEEP,
};

struct ProcessMemory {
  unsigned long long userStackPageFrame;
  unsigned long long kernelStackPageFrame;
  unsigned long long textSegmentPageFrame;
  struct L4PTEntry *l4PageTableBase;
};

struct Process;
struct ThreadInfo {
  struct Registers registers;
  struct Process *process;
};

struct Process {
  int pid;
  enum ProcessState state;
  struct Process *parent;
  struct List processes;
  struct ThreadInfo *threadInfo;
  struct ProcessMemory *processMemory;
};

extern struct Process *currentProcess;
extern struct List processList;

void exitHandler(unsigned long long status);
void KernelThread(unsigned long long fn);
void ProcessInit();
int sysExec(unsigned long long rsp, char *filename);
int sysFork(unsigned long long rsp);
#endif
