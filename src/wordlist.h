#ifndef WORDLIST_H
#define WORDLIST_H

#include "sl.h"
#include "stdio.h"

typedef struct {
  const char *chars;
  const SL *words;
  long nwords;
  long nchars;
} WordList;

void exit_err_file(const char *msg, const char *fname);
long get_fsize_or_panic(FILE *f, const char *fname);

/**
 * @brief get WordList from words in a file
 */
WordList get_malloced_wordlist(const char *fname);

/**
 * @brief free malloced WordList data
 */
void WL_free(WordList wl);

#endif // WORDLIST_H
