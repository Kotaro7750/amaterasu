/**
 * @file util.c
 * @brief 汎用的に使える関数群
 */
#include "include/util.h"

// TODO return value
int strncmp(char *a, char *b, int length) {
  for (int i = 0; i < length; i++) {
    if (a[i] != b[i]) {
      return 1;
    }
  }
  return 0;
}

int strnlength(char *a) {
  int length = 0;
  while (a[length] != 0) {
    length++;
  }

  return length;
}

unsigned long long pow(unsigned long long x, unsigned long long n) {
  unsigned long long ret = 1;
  while (n != 0) {
    if (n & 1) {
      ret *= x;
    }
    x *= x;
    n >>= 1;
  }
  return ret;
}

unsigned long long log2(unsigned long long x) {
  unsigned long long ret = 0;
  while (x != 1) {
    x /= 2;
    ret++;
  }
  return ret;
}

void memcpy(void *dst, void *src, unsigned long long size) {
  for (unsigned long long i = 0; i < size; i++) {
    *((char *)dst + i) = *(char *)(src + i);
  }
}

void memset(void *addr, char value, unsigned long long size) {
  for (unsigned long long i = 0; i < size; i++) {
    *(char *)(addr + i) = value;
  }
}
