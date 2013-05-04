#include <stdio.h>

#define SHA1_DIGEST_LENGTH 2

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


int main() {
  
  /*
  // If these are tested, the SHA1_DIGEST_LENGTH has to be changed to 20
  unsigned char key1[SHA1_DIGEST_LENGTH] =
                {0xd4, 0xff, 0xf2, 0x36, 0x38, 0x60, 0xf0, 0xab, 0x21, 0xac,
                 0x39, 0x85, 0x31, 0x98, 0xcd, 0xa2, 0xa1, 0x77, 0x6b, 0x00};
  unsigned char key2[SHA1_DIGEST_LENGTH] =
                {0x9b, 0xd6, 0xc8, 0xa6, 0x0a, 0xfc, 0xf0, 0x53, 0xc1, 0xda,
                 0x3e, 0x90, 0xf9, 0x0d, 0x79, 0x5a, 0x7c, 0xa0, 0xf3, 0xfb};
  unsigned char target[SHA1_DIGEST_LENGTH] =
                {0xd6, 0xef, 0xb8, 0xd1, 0x66, 0x52, 0x24, 0xe7, 0x79, 0x33,
                 0x5a, 0x40, 0x05, 0x42, 0x86, 0x53, 0x62, 0x2e, 0xad, 0xb8};
  */
  
  /*
  unsigned char key1[SHA1_DIGEST_LENGTH] = {0xf2, 0xa1, 0x15, 0x12, 0xf0};
  unsigned char key2[SHA1_DIGEST_LENGTH] = {0x72, 0xa1, 0x15, 0x12, 0xef};
  */
  
  unsigned char key1[SHA1_DIGEST_LENGTH] =   {0xff, 0xff};
  unsigned char key2[SHA1_DIGEST_LENGTH] =   {0x00, 0x01};
  unsigned char target[SHA1_DIGEST_LENGTH] = {0x80, 0x00};
  
  printf("Key1: ");
  for (int i = 0; i < SHA1_DIGEST_LENGTH; i++)
    printf("%02x ", key1[i]);
  printf("\n");
  printf("Key2: ");
  for (int i = 0; i < SHA1_DIGEST_LENGTH; i++)
    printf("%02x ", key2[i]);
  printf("\n");
  printf("Target: ");
  for (int i = 0; i < SHA1_DIGEST_LENGTH; i++)
    printf("%02x ", target[i]);
  printf("\n");
  printf("\n");
  
  unsigned char result[SHA1_DIGEST_LENGTH];
  difference(result, key1, target);
  printf("Dif1: ");
  for (int i = 0; i < SHA1_DIGEST_LENGTH; i++)
    printf("%02x ", result[i]);
  printf("\n");
  printf("Dif2: ");
  difference(result, key2, target);
  for (int i = 0; i < SHA1_DIGEST_LENGTH; i++)
    printf("%02x ", result[i]);
  printf("\n");
  printf("\n");
  
  int ret = find_closer_key(target, key1, key2);
  if (ret < 0)
    printf("Equal keys");
  else if (ret)
    printf("Key1 closer");
  else
    printf("Key2 closer");
  printf("\n");
  
  return 0;
}
