#include "errno.h"
#include "stdio.h"
#include "stdlib.h"

#define SL_IMPLEMENTATION
#include "sl.h"

typedef struct {
  const char *chars;
  const SL *words;
  long nwords;
  long nchars;
} WordList;

void WL_free(WordList wl) {
  free((void*)wl.chars);
  free((void*)wl.words);
}

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

const WordList get_malloced_wordlist(const char *fname) {
  FILE *f = fopen(fname, "r");

  if (errno) {
    fprintf(stderr, "Error opening wordlist file '%s': %s\nExiting...\n", fname,
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  const long fsize = get_fsize_or_panic(f, fname);

  char *chars = malloc(fsize * sizeof(char));
  fread(chars, fsize, 1, f);
  exit_err_file("Error reading file", fname);

  fclose(f);
  exit_err_file("Error closing file", fname);

  WordList ret = {
      .nchars = fsize,
      .chars = chars,
  };

  SL allchars = {ret.chars, fsize};

  long word_count = 1;
  for (long pos = 0; pos < ret.nchars; ++pos) {
    if (ret.chars[pos] == '\n' || ret.chars[pos] == ' ') {
      ++word_count;
    }
  }

  SL *words = malloc(word_count * sizeof(SL));

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

  ret.words = realloc(words, (word_count - invalid_words) * sizeof(SL));
  ret.nwords = word_count - invalid_words;

  return ret;
}

int main() {
  WordList w_list = get_malloced_wordlist("./top1000en.txt");

  for (long wi = 0; wi < w_list.nwords; ++wi) {
    printf("%ld: " SL_FMT "\n", wi, SL_FP(w_list.words[wi]));
  }

  WL_free(w_list);
  return EXIT_SUCCESS;
}
