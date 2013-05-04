#ifndef SHA1_H_INCLUDED
#define SHA1_H_INCLUDED

#include <string.h>

// Include CommonCrypto or OpenSSL based on operating system
#ifdef __APPLE__
  #include <CommonCrypto/CommonDigest.h>
  #define SHA1_DIGEST_LENGTH CC_SHA1_DIGEST_LENGTH
  void sha1(unsigned char *source, unsigned int len, unsigned char *target);
#else
  #include <openssl/sha.h>
  #define SHA1_DIGEST_LENGTH SHA_DIGEST_LENGTH
  void sha1(unsigned char *source, unsigned int len, unsigned char *target);
#endif

// Places the modular difference between keys a and b into array result.
// All arrays must be of length SHA1_DIGEST_LENGTH.
// Retur: 1 if a >= b, 0 otherwise
int difference(unsigned char *result, unsigned char *a, unsigned char *b);

// Check which of keys a and b is closer to the target key. If they are equally
// close, the next key from the target to the incrementing direction is
// considered closer. All arrays must be of length SHA1_DIGEST_LENGTH.
// Return: 1 if a is closer, 0 if b is closer, -1 if a and b are equal.
int find_closer_key(unsigned char *target, unsigned char *a, unsigned char *b);

// Utility function for checking if two SHA1 keys are equal
// Return: 1 if equal, 0 if not equal
int compare_keys(unsigned char *key1, unsigned char *key2);

#endif
