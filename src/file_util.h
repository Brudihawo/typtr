#ifndef FILE_UTIL_H
#define FILE_UTIL_H
#include "stdio.h"
#include "stdlib.h"

void exit_err_file(const char *msg, const char *fname);

long get_fsize_or_panic(FILE *f, const char *fname);
#endif
