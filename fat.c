/**
 * @file fat.c
 * @brief FATファイルシステムの処理
 */
#include "include/fat.h"
#include "include/ata.h"
#include "include/graphic.h"
#include "include/syscall.h"

struct Drive drives[4];

void DriveInit() {
  for (int i = 0; i < 4; i++) {
    if (drives[i].isValid) {
      unsigned char buffer[512];
      int pushedIndex = Syscall(SYSCALL_READ, 0, (unsigned long long)buffer, 0);

      while (ATARequestComplete(pushedIndex) == 0) {
        ;
      }

      drives[i].MBR.id = *(unsigned int *)(buffer + 0x1b8);
      for (int partition = 0; partition < 4; partition++) {
        unsigned long long partitionTableStart = (unsigned long long)buffer + 0x1be + 16 * partition;
        if (*(unsigned char *)partitionTableStart != 0) {
          drives[i].MBR.partitionTables[partition].isValid = 1;
          drives[i].MBR.partitionTables[partition].startLBA = *(unsigned int *)(partitionTableStart + 0x08);
          drives[i].MBR.partitionTables[partition].numbersOfSectors = *(unsigned int *)(partitionTableStart + 0x0c);
        }
      }
    }
  }

  unsigned char buffer[512];
  int pushedIndex = Syscall(SYSCALL_READ, drives[0].MBR.partitionTables[0].startLBA, (unsigned long long)buffer, 0);

  while (ATARequestComplete(pushedIndex) == 0) {
    ;
  }

  for (int i = 0; i<512; i++) {
    if (buffer[i] != 0) {
      puth(i);
      puts(" ");
      puth(buffer[i]);
      puts("\n");
    }
  }
}
