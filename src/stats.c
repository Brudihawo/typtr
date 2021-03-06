#include "stats.h"

#include "text.h"

#include "assert.h"
#include "errno.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include <stdint.h>

uint32_t *crc_table;


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

void deinit_crc_table(void) { free(crc_table); }

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
    const int correct_idx = char_idx(t->chars[i]);
    const int actual_idx = char_idx(t->typedchars[i]);
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
        printf("%3ld ", confusion);
      } else {
        printf("    ");
      }
    }
    printf("\n");
  }
  printf("\n");
}

void MDS_update(MonoGramDataSummary *mds, Text *t) {
  for (int i = 0; i < N_CHARS; ++i) {
    mds->times[i] *= mds->n_occurrences[i];
  }

  for (int i = 0; i < t->n_chars; ++i) {
    const int chr_idx = char_idx(t->chars[i]);
    if (t->chars[i] != t->typedchars[i]) {
      ++mds->n_misses[chr_idx];
    }
    ++mds->n_occurrences[chr_idx];
    mds->times[chr_idx] += t->time_to_type[i];
  }
  for (int i = 0; i < N_CHARS; ++i) {
    mds->times[i] /= (float)mds->n_occurrences[i];
  }
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

void BT_update(BigramTable *b, const Text *t) {
  for (long i = 0; i < t->n_chars - 1; ++i) {
    const int c1no_act = char_idx(t->chars[i]);
    const int c2no_act = char_idx(t->chars[i + 1]);
    const int c1no_typ = char_idx(t->typedchars[i]);
    const int c2no_typ = char_idx(t->typedchars[i + 1]);
    const float t1 = t->time_to_type[i];
    const float t2 = t->time_to_type[i + 1];

    b->avg_execution_time[c1no_act][c2no_act] *=
        b->n_occurrences[c1no_act][c2no_act];

    b->avg_execution_time[c1no_act][c2no_act] += t1 + t2;

    ++b->n_occurrences[c1no_act][c2no_act];
    b->avg_execution_time[c1no_act][c2no_act] /=
        b->n_occurrences[c1no_act][c2no_act];

    if (c1no_act != c1no_typ || c2no_act != c2no_typ) {
      ++b->n_misses[c1no_act][c2no_act];
    }
  }
}

void dump_stats_bin(FILE *f, const MonoGramDataSummary *mds,
                    const ConfMatrix *confusions, const BigramTable *bt) {
  fwrite(confusions, sizeof(ConfMatrix), 1, f);
  fwrite(mds, sizeof(MonoGramDataSummary), 1, f);
  fwrite(bt, sizeof(BigramTable), 1, f);
}

void dump_stats_csv(const MonoGramDataSummary *mds,
                    const ConfMatrix *confusions, const BigramTable *bt) {
  FILE *conf_file = fopen("./confusions.csv", "w");

  fprintf(conf_file, "correct");
  for (int i = 0; i < N_CHARS; ++i) {
    fprintf(conf_file, ",%i", i + 32);
  }
  fprintf(conf_file, "\n");

  for (int i = 0; i < N_CHARS; ++i) {
    fprintf(conf_file, "%i", i + 32);
    for (int j = 0; j < N_CHARS; ++j) {
      fprintf(conf_file, ",%ld", confusions->matrix[i][j]);
    }
    fprintf(conf_file, "\n");
  }
  fclose(conf_file);

  FILE *mds_file = fopen("./mds.csv", "w");
  fprintf(mds_file, "char,occurrences,misses,avg_time\n");
  for (int i = 0; i < N_CHARS; ++i) {
    fprintf(mds_file, "%i,%ld,%ld,%f\n", i + 32, mds->n_occurrences[i],
            mds->n_misses[i], mds->times[i]);
  }
  fclose(mds_file);

  FILE *bt_file = fopen("./bigramtable.csv", "w");
  fclose(bt_file);
}
