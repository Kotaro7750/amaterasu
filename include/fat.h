/**
 * @file fat.h
 * @brief FATファイルシステムのヘッダ
 */

#ifndef _FAT_H_
#define _FAT_H_

#include <file.h>

/**
 * @brief MBRのパーティションテーブルエントリ
 */
struct PartitionTableEntry {
  unsigned char isValid;
  unsigned int startLBA;
  unsigned int numbersOfSectors;
};

/**
 * @brief マスターブートレコード
 */
struct MasterBootRecord {
  unsigned int id;
  struct PartitionTableEntry partitionTables[4];
};

struct Drive {
  unsigned char isValid;
  struct MasterBootRecord MBR;
};

struct FAT {
  unsigned int startLBA;
};

struct Partition {
  unsigned int startLBA;
  unsigned int numbersOfSectors;
  unsigned char sectorsOfCluster;
  unsigned short numbersOfReservedSectors;
  unsigned char numbersOfFATs;
  unsigned short sectorsOfFAT;
  struct FAT FATs[2];
  unsigned short numbersOfRootDirEntries;
  unsigned int rootDirStartLBA;
  unsigned int sectorsOfRootDir;
  unsigned int dataStartLBA;
};

extern struct Drive drives[4];
extern struct Partition partitions[1];

void DriveInit();
void ParseBootRecord(unsigned char BRsector[512], unsigned int startLBA);
#endif
