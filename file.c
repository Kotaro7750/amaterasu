/**
 * @file file.c
 * @brief ファイルの処理
 */
#include <ata.h>
#include <fat.h>
#include <file.h>
#include <util.h>

int GetFileInfo(char *filename, struct File *file, char enablePoll) {
  // TODO ディレクトリエントリが複数のセクタにまたがるときの対応
  unsigned char buffer[512];
  ATARead(partitions[0].rootDirStartLBA, 512, buffer, enablePoll);

  int length = strnlength(filename);

  for (int i = 0; i < 512 / 32; i++) {
    if (buffer[32 * i] != 0x00) {
      if (strncmp(filename, (char *)(buffer + 32 * i), length) == 0) {
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

int ReadFile(struct File *file, unsigned char *buffer, char enablePoll) { return ATARead(file->start, file->size, buffer, enablePoll); }
