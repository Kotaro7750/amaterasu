/**
 * @file main.c
 * @brief カーネルのメイン処理
 */

#include <acpi.h>
#include <ata.h>
#include <fat.h>
#include <fb.h>
#include <file.h>
#include <graphic.h>
#include <hpet.h>
#include <interrupt.h>
#include <kHeap.h>
#include <kbc.h>
#include <list.h>
#include <paging.h>
#include <physicalMemory.h>
#include <pic.h>
#include <process.h>
#include <scheduler.h>
#include <syscall.h>
#include <x86_64.h>

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
    Syscall(SYSCALL_PUT, 'B', 0, 0);
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
  GraphicInit();
  PhysicalMemoryManagementInit(*freeMapInfo);
  kHeapInit();
  gdtInit();
  ProcessInit();

  idtInit();
  picInit();
  KBCInit();
  ACPIInit(_pi->RSDPAddress);
  HPETInit();
  SyscallInit();
  ATAInit();

  PagingInit();

  EnableCPUInterrupt();

  DriveInit();
  KernelThread((unsigned long long)taskC);
  SchedulerStart();

  Syscall(SYSCALL_EXEC, "TEST", 0, 0);

  while (1)
    CpuHalt();
}
