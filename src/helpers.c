#include "helpers.h"

// Utility method for printing error messages and exiting
void die(char *reason)
{
  fprintf(stderr, "Fatal error: %s\n", reason);
  exit(1);
}
