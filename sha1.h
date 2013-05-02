#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#include <string.h>

// Include CommonCrypto or OpenSSL based on operating system
#ifdef __APPLE__
  #include <CommonCrypto/CommonDigest.h>
  #define SHA1_DIGEST_LENGTH CC_SHA1_DIGEST_LENGTH
#else
  #include <openssl/sha.h>
  #define SHA1_DIGEST_LENGTH SHA_DIGEST_LENGTH
#endif

char *sha1(char value[]);

#endif
