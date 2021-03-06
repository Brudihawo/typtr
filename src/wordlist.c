#include "wordlist.h"
#include "assert.h"
#include "errno.h"
#include "file_util.h"
#include "sl.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>

void WL_free(WordList wl) {
  free((void *)wl.chars);
  free((void *)wl.words);
}

WordList get_malloced_wordlist(const char *fname) {
  errno = 0;
  FILE *f = fopen(fname, "r");

  if (errno) {
    fprintf(stderr, "Error opening wordlist file '%s': %s\nExiting...\n", fname,
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  const long fsize = get_fsize_or_panic(f, fname);

  assert(fsize > 0);
  char *chars = malloc((unsigned long)fsize * sizeof(char));
  fread(chars, (unsigned long)fsize, 1, f);
  exit_err_file("Error reading file", fname);

  fclose(f);
  exit_err_file("Error closing file", fname);

  WordList ret = {
      .nchars = fsize,
      .chars = chars,
  };

  long word_count = 1;
  for (long pos = 0; pos < ret.nchars; ++pos) {
    if (ret.chars[pos] == '\n' || ret.chars[pos] == ' ') {
      ++word_count;
    }
  }

  SL *words = malloc((unsigned long)word_count * sizeof(SL));

  long cur_word_start = 0;
  long invalid_words = 0;
  for (long wi = 0; wi < word_count; ++wi) {
    long word_length = 0;
    while (!(ret.chars[cur_word_start + word_length] == '\n' ||
             ret.chars[cur_word_start + word_length] == ' ')) {
      ++word_length;
      if (cur_word_start + word_length >= ret.nchars)
        break;
    }

    if (word_length > 0) {
      words[wi - invalid_words] =
          (SL){.start = &ret.chars[cur_word_start], .len = word_length};
      cur_word_start += word_length + 1;
    } else {
      ++invalid_words;
    }
  }

  ret.words =
      realloc(words, (unsigned long)(word_count - invalid_words) * sizeof(SL));
  ret.nwords = word_count - invalid_words;

  return ret;
}

void WL_deepcopy(const WordList* src, WordList* dst) {
  dst->nchars = src->nchars;
  dst->nwords = src->nwords;
  SL* words = malloc((unsigned long)src->nwords * sizeof(SL));
  char* chars = malloc((unsigned long)src->nchars * sizeof(char));

  memcpy(words, src->words, (unsigned long)src->nwords * sizeof(SL));
  dst->words = words;

  memcpy(chars, src->chars, (unsigned long)dst->nchars);
  dst->chars = chars;
}

WordList WL_sample(const WordList *wl, const long *idcs, long n_words) {
  assert(n_words <= wl->nwords);

  WordList ret = {0};
  ret.nwords = n_words;

  for (long i = 0; i < n_words; ++i) {
    ret.nchars += wl->words[idcs[i]].len;
  }

  char* chars = malloc((unsigned long)ret.nchars * sizeof(char));
  SL *words = malloc((unsigned long)n_words * sizeof(SL));
  long cur_idx = 0;
  for (long i = 0; i < n_words; ++i) {
    words[i] = wl->words[idcs[i]];
    memcpy(&chars[cur_idx], &words[i].start,
           (unsigned long)words[i].len);
    cur_idx += words[i].len;
  }
  ret.chars = chars;
  ret.words = words;
  return ret;
}
