#include "file_util.h"

#include "errno.h"
#include "stdlib.h"
#include "string.h"

void exit_err_file(const char *msg, const char *fname) {
  if (errno) {
    fprintf(stderr, "%s '%s': %s\nExiting...\n", msg, fname, strerror(errno));
    exit(EXIT_FAILURE);
  }
}

long get_fsize_or_panic(FILE *f, const char *fname) {
  long init_pos = ftell(f);
  if (init_pos < 0) {
    exit_err_file("Error getting word list file size", fname);
  }

  fseek(f, 0, SEEK_END);
  exit_err_file("Error getting word list file size", fname);

  long size = ftell(f);

  if (size < 0) {
    exit_err_file("Error getting word list file size", fname);
  }

  fseek(f, init_pos, SEEK_SET);

  return size;
}
