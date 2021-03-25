/**
 * @file fat.c
 * @brief FATファイルシステムの処理
 */
#include <fat.h>
#include <ata.h>
#include <file.h>
#include <graphic.h>
#include <pic.h>
#include <scheduler.h>
#include <syscall.h>
#include <util.h>

struct Drive drives[4];
struct Partition partitions[1];

void DriveInit() {
  for (int i = 0; i < 4; i++) {
    if (drives[i].isValid) {
      unsigned char buffer[512];
      Syscall(SYSCALL_ATA_READ, 0, 512, (unsigned long long)buffer);

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
  Syscall(SYSCALL_ATA_READ, drives[0].MBR.partitionTables[0].startLBA, 512, (unsigned long long)buffer);

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
