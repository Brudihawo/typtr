#include "sl.h"
#include "stdio.h"

typedef struct {
  const char *chars;
  const SL *words;
  long nwords;
  long nchars;
} WordList;

void WL_free(WordList wl);

void exit_err_file(const char *msg, const char *fname);

long get_fsize_or_panic(FILE *f, const char *fname);

WordList get_malloced_wordlist(const char *fname);
