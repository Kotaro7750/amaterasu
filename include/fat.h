/**
 * @file fat.h
 * @brief FATファイルシステムのヘッダ
 */

#ifndef _FAT_H_
#define _FAT_H_

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

extern struct Drive drives[4];

void DriveInit();
#endif
