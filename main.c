/**
 * @file main.c
 * @brief カーネルのメイン処理
 */

#include "include/acpi.h"
#include "include/ata.h"
#include "include/fat.h"
#include "include/fb.h"
#include "include/graphic.h"
#include "include/hpet.h"
#include "include/interrupt.h"
#include "include/kHeap.h"
#include "include/kbc.h"
#include "include/paging.h"
#include "include/physicalMemory.h"
#include "include/pic.h"
#include "include/scheduler.h"
#include "include/syscall.h"
#include "include/x86_64.h"

/**
 * @struct PlatformInfo
 * @brief ブートローダーから渡される情報
 */
struct __attribute__((packed)) PlatformInfo {
  //! フレームバッファの情報
  struct FrameBuffer fb;
  //! RSDPのアドレス
  void *RSDPAddress;
};

/**
 * @brief Aを出力し続ける
 */
void taskA() {
  while (1) {
    putc('A');
    volatile unsigned long long wait = 10000000;
    while (wait--)
      ;
  }
}

/**
 * @brief Bを出力し続ける
 */
void taskB() {
  while (1) {
    putc('B');
    volatile unsigned long long wait = 10000000;
    while (wait--)
      ;
  }
}

/**
 * @brief Cを出力し続ける
 */
void taskC() {
  while (1) {
    putc('C');
    volatile unsigned long long wait = 10000000;
    while (wait--)
      ;
  }
}

/**
 * @brief カーネルのスタートアップルーチン
 * @param[in] _t 未使用
 * @param[in] _pi ブートローダーから渡されるハードウェア関係情報
 * @param[in] freeMapInfo ブートローダーから渡される物理メモリマップ情報
 */
void start_kernel(void *_t __attribute__((unused)), struct PlatformInfo *_pi, struct PhysicalMemoryFreeMapInfo *freeMapInfo) {
  FBInit(&(_pi->fb));
  gdtInit();
  GraphicInit();

  idtInit();
  picInit();
  KBCInit();
  ACPIInit(_pi->RSDPAddress);
  HPETInit();
  SyscallInit();
  SchedulerInit();
  ATAInit();

  PhysicalMemoryManagementInit(*freeMapInfo);
  PagingInit();
  kHeapInit();

  EnableCPUInterrupt();

  // Syscall(SYSCALL_EXEC, (unsigned long long)taskA, 0, 0);
  // Syscall(SYSCALL_EXEC, (unsigned long long)taskB, 0, 0);
  // Syscall(SYSCALL_EXEC, (unsigned long long)taskC, 0, 0);

  // Schedule(0);
  // SchedulerStart();

  // DriveInit();
  
  unsigned long long addr1 = kmalloc(256*2);
  DumpkHeap();
  puts("\n");

  unsigned long long addr2 = kmalloc(256);
  DumpkHeap();
  puts("\n");

  kfree(addr1);
  DumpkHeap();
  puts("\n");

  kfree(addr1);
  DumpkHeap();
  puts("\n");

  while (1)
    CpuHalt();
}
