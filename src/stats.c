#include "stats.h"

#include "text.h"

#include "assert.h"
#include "errno.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include <stdint.h>

const char keys[N_CHARS] = "abcdefghijklmnopqrstuvwxyz ";
uint32_t* crc_table;

void init_crc_table(void) {
  crc_table = malloc(256 * sizeof(uint32_t));
  unsigned long poly = 0xEDB88320;
  unsigned long remainder;
  unsigned char b = 0;
  do {
    remainder = b;
    for (unsigned long bit = 8; bit > 0; --bit) {
      if (remainder & 1) {
        remainder = (remainder >> 1) ^ poly;
      } else {
        remainder = remainder >> 1;
      }
    }
    crc_table[b] = remainder;
  } while (0 != ++b);
}

void deinit_crc_table(void) {
  free(crc_table);
}

uint32_t crc32(const char *bytes, long size) {
  uint32_t ret = 0xFFFFFFFF;
  for (long i = 0; i < size; ++i) {
    const long lookup_idx = (ret ^ (uint32_t)bytes[i]) & 0xFF;
    ret = (ret >> 8) ^ crc_table[lookup_idx];
  }
  return ret ^ 0xFFFFFFFF;
}

void update_conf_matrix(ConfMatrix *mat, Text *t) {
  mat->n_hits = t->n_chars;
  for (int i = 0; i < t->n_chars; ++i) {
    const int correct_idx = t->chars[i] == ' ' ? 26 : t->chars[i] - 97;
    const int actual_idx = t->typedchars[i] == ' ' ? 26 : t->typedchars[i] - 97;
    ++mat->matrix[correct_idx][actual_idx];
  }
}

void print_conf_matrix(ConfMatrix *mat) {
  printf("   ");
  for (int i = 0; i < N_CHARS; ++i) {
    printf(" %c  ", keys[i]);
  }
  printf("\n");

  for (int i = 0; i < N_CHARS; ++i) {
    printf(" %c ", keys[i]);
    for (int j = 0; j < N_CHARS; ++j) {
      const long confusion = mat->matrix[i][j];
      if (confusion > 0) {
        printf("%2ld ", confusion);
      } else {
        printf("    ");
      }
    }
    printf("\n");
  }
  printf("\n");
}

MonoGramDataSummary build_monogram_data(Text *t) {
  MonoGramDataSummary mds = {0};
  for (int i = 0; i < t->n_chars; ++i) {
    const int char_idx = t->chars[i] == ' ' ? 26 : t->chars[i] - 97;
    if (t->chars[i] != t->typedchars[i]) {
      ++mds.n_misses[char_idx];
    }
    ++mds.n_occurrences[char_idx];
    mds.times[char_idx] += t->time_to_type[i];
  }
  for (int i = 0; i < N_CHARS; ++i) {
    mds.times[i] /= (float)mds.n_occurrences[i];
  }
  return mds;
}

void print_mds(MonoGramDataSummary *mds) {
  for (int i = 0; i < N_CHARS; ++i) {
    printf("%5.1f ", mds->times[i]);
  }
  printf("\n");
  for (int i = 0; i < N_CHARS; ++i) {
    printf("%5ld ", mds->n_occurrences[i]);
  }
  printf("\n");
  for (int i = 0; i < N_CHARS; ++i) {
    printf("%5ld ", mds->n_misses[i]);
  }
  printf("\n");
}

void dump_stats(FILE *f, const MonoGramDataSummary *mds,
                const ConfMatrix *confusions) {
  fwrite(confusions, sizeof(ConfMatrix), 1, f);
  fwrite(mds, sizeof(MonoGramDataSummary), 1, f);
}
