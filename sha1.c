#include "sha1.h"

// Include CommonCrypto or OpenSSL based on operating system
#ifdef __APPLE__
  void sha1(unsigned char *source, unsigned int len, unsigned char *target) {
    CC_SHA1(source, len, target);
  }
#else
  void sha1(unsigned char *source, unsigned int len, unsigned char *target) {
    SHA1(source, len, target);
  }
#endif

// Places the modular difference between keys a and b into array result.
// All arrays must be of length SHA1_DIGEST_LENGTH.
// Retur: 1 if a >= b, 0 otherwise
int difference(unsigned char *result, unsigned char *a, unsigned char *b)
{
  // Check if a is bigger than b. If not -> switch
  int a_bigger = 1;
  for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
    if (b[i] > a[i]) {
      unsigned char *temp = a;
      a = b;
      b = temp;
      a_bigger = 0;
      break;
    } else if (b[i] < a[i])
      break;
  }
  // Perform subtraction a - b
  short carry = 0;
  short v1;
  for (int i = SHA1_DIGEST_LENGTH - 1; i >= 0; i--) {
    v1 = (short)a[i] - carry - (short)b[i];
    carry = 0;
    if (v1 < 0){
      carry = 1;
      v1 += 0x10000;
    }
    result[i] = v1 & 0xff;
  }
  // Check if the complement distance is closer i.e. smaller than 0x80..0
  unsigned short limit = 0x80;
  for (int j = 0; j < SHA1_DIGEST_LENGTH; j++) {
    if ( result[j] > limit ) {
      // Complement is closer
      for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
        result[i] = ~result[i];
      }
      for (int i = SHA1_DIGEST_LENGTH - 1; i >= 0; i--) {
        if (result[i] != 0xff) {
          result[i] += 1;
          break;
        }
        result[i] = 0;
      }
      break;
    } else if (result[j] < limit)
      break;
    if (!j) limit = 0;
  }
  return a_bigger;
}

// Check which of keys a and b is closer to the target key. If they are equally
// close, the next key from the target to the incrementing direction is
// considered closer. All arrays must be of length SHA1_DIGEST_LENGTH.
// Return: 1 if a is closer, 0 if b is closer, -1 if a and b are equal.
int find_closer_key(unsigned char *target, unsigned char *a, unsigned char *b)
{
  // Calculate differences
  unsigned char dif_a[SHA1_DIGEST_LENGTH];
  int a_bigger_t = difference(dif_a, a, target);
  unsigned char dif_b[SHA1_DIGEST_LENGTH];
  int b_bigger_t = difference(dif_b, b, target);
  // Check if one is closer
  for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
    if (dif_b[i] > dif_a[i])
      return 1;
    if (dif_b[i] < dif_a[i])
      return 0;
  }
  // Distances are equal. Checking which one is in incremental direction.
  // N.B. in this point a and b cant be equal to the target.
  if (a_bigger_t && !b_bigger_t)
    return 1;
  if (!a_bigger_t && b_bigger_t)
    return 0;
  // Check if a or b is bigger
  for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
    if (b[i] > a[i]) {
      return 1;
    } else if (b[i] < a[i])
      return 0;
  }
  // a and b are equal
  return -1;
}

// Utility function for checking if two SHA1 keys are equal
// Return: 1 if equal, 0 if not equal
int compare_keys(unsigned char *key1, unsigned char *key2)
{
  for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
    if (key1[i] != key2[i])
      return 0;
  }
  return 1;
}

