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
