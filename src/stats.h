#ifndef STATS_H
#define STATS_H

#include "stdint.h"
#include "text.h"

#define N_CHARS (128 - 32 - 1)
#define STORAGE_NAME "typtr_data.dat"

extern const char keys[N_CHARS];
extern uint32_t *crc_table;

typedef struct {
  long matrix[N_CHARS][N_CHARS];
  long n_hits;
} ConfMatrix;

typedef struct {
  float times[N_CHARS];
  long n_occurrences[N_CHARS];
  long n_misses[N_CHARS];
} MonoGramDataSummary;

typedef struct {
  float avg_execution_time[N_CHARS][N_CHARS];
  long n_occurrences[N_CHARS][N_CHARS];
  long n_misses[N_CHARS][N_CHARS];
} BigramTable;

void update_conf_matrix(ConfMatrix *mat, Text *t);

void print_conf_matrix(ConfMatrix *mat);

MonoGramDataSummary build_monogram_data(Text *t);

void print_mds(MonoGramDataSummary *mds);

void dump_stats(FILE *f, const MonoGramDataSummary *mds,
                const ConfMatrix *confusions, const BigramTable *bt);

// CRC for generating random seed
void init_crc_table(void);
void deinit_crc_table(void);
uint32_t crc32(const char *bytes, long size);

void BT_update(BigramTable *b, const Text *t);
#endif // STATS_H
