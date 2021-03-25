#ifndef _UTIL_H_
#define _UTIL_H_

int strncmp(char *a, char *b, int length);
int strnlength(char *a);
unsigned long long pow(unsigned long long x, unsigned long long n);
unsigned long long log2(unsigned long long x);
void memcpy(void *dst, void *src, unsigned long long size);
void memset(void *addr, char value, unsigned long long size);
#endif
