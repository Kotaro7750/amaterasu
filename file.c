/**
 * @file file.c
 * @brief ファイルの処理
 */
#include "include/file.h"
#include "include/ata.h"
#include "include/fat.h"
#include "include/util.h"

int GetFileInfo(char *filename, struct File *file) {
  // TODO ディレクトリエントリが複数のセクタにまたがるときの対応
  unsigned char buffer[512];
  ATARead(partitions[0].rootDirStartLBA, 512, buffer);

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

int ReadFile(struct File *file, unsigned char *buffer) { return ATARead(file->start, file->size, buffer); }
