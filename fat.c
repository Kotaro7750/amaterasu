/**
 * @file fat.c
 * @brief FATファイルシステムの処理
 */
#include "include/fat.h"
#include "include/ata.h"
#include "include/file.h"
#include "include/pic.h"
#include "include/graphic.h"
#include "include/scheduler.h"
#include "include/syscall.h"
#include "include/util.h"

struct Drive drives[4];
struct Partition partitions[1];

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

  ParseBootRecord(buffer, drives[0].MBR.partitionTables[0].startLBA);
}

void ParseBootRecord(unsigned char BRsector[512], unsigned int startLBA) {
  unsigned short reservedSectors = *(unsigned short *)(BRsector + 0x0e);
  unsigned short sectorsOfFAT = *(unsigned short *)(BRsector + 0x16);
  unsigned char numbersOfFATs = *(unsigned char *)(BRsector + 0x10);

  partitions[0].startLBA = startLBA;
  partitions[0].sectorsOfCluster = *(unsigned char *)(BRsector + 0x0d);
  partitions[0].numbersOfReservedSectors = reservedSectors;

  partitions[0].numbersOfFATs = numbersOfFATs;
  partitions[0].sectorsOfFAT = sectorsOfFAT;
  for (int i = 0; i < 2; i++) {
    partitions[0].FATs[i].startLBA = startLBA + reservedSectors + sectorsOfFAT * i;
  }

  unsigned short numbersOfRootDirEntries = *(unsigned short *)(BRsector + 0x11);
  partitions[0].numbersOfRootDirEntries = numbersOfRootDirEntries;
  partitions[0].sectorsOfRootDir = (numbersOfRootDirEntries * 32 + 511) / 512;

  partitions[0].dataStartLBA = startLBA + reservedSectors + (sectorsOfFAT * numbersOfFATs) + partitions[0].sectorsOfRootDir;

  partitions[0].rootDirStartLBA = partitions[0].dataStartLBA - partitions[0].sectorsOfRootDir;

  partitions[0].numbersOfSectors = *(unsigned int *)(BRsector + 0x20);
}

int GetFileInfo(char *filename, struct File *file) {
  unsigned char buffer[512];
  int pushedIndex = ATARead(partitions[0].rootDirStartLBA, buffer);

  while (ATARequestComplete(pushedIndex) == 0) {
    ;
  }

  int length = strnlength(filename);

  for (int i = 0; i < 512 / 32; i++) {
    if (buffer[32 * i] != 0x00) {
      if (strncmp(filename, (char *)(buffer + 32 * i), strnlength(filename)) == 0) {
        unsigned short clusterHead = *(unsigned short *)(buffer + 32 * i + 26);
        unsigned int filesize = *(unsigned int *)(buffer + 32 * i + 28);

        file->start = partitions[0].dataStartLBA + partitions[0].sectorsOfCluster * (clusterHead - 2);
        file->size = filesize;

        return 1;
      }
    }
  }

  return 0;
}
