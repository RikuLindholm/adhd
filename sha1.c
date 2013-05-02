#include "sha1.h"

#ifdef __APPLE__
  char *sha1(char value[]) {
    static char hash[40];
    int len = strlen(value);
    CC_SHA1(value, len, (unsigned char *)hash);
    return hash;
  }
#else
  char *sha1(char value[]) {
    static char hash[40];
    int len = strlen(value);
    SHA1(value, len, (unsigned char *)hash);
    return hash;
  }
#endif
