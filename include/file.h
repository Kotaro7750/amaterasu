#ifndef _FILE_H_
#define _FILE_H_

struct File {
  unsigned long long start;
  unsigned long long size;
};

int GetFileInfo(char *filename, struct File *file, char enablePoll);
int ReadFile(struct File *file, unsigned char *buffer, char enablePoll);
#endif
