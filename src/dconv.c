#include "stats.h"
#include "errno.h"
#include "stdlib.h"
#include <stdio.h>

int main(void) {
  ConfMatrix confusions = {0};
  BigramTable bt = {0};
  MonoGramDataSummary mds = {0};

  FILE *data_file = fopen(STORAGE_NAME, "r");
  if (errno) {
    if (errno == ENOENT) {
      // file does not exist
    } else {
      fprintf(stderr, "Error opening storage file '%s': %s\nExiting...\n",
              STORAGE_NAME, strerror(errno));
      exit(EXIT_FAILURE);
    }
  } else {
    fseek(data_file, 0, SEEK_SET);
    fread(&confusions, sizeof(ConfMatrix), 1, data_file);
    fread(&mds, sizeof(MonoGramDataSummary), 1, data_file);
    fread(&bt, sizeof(BigramTable), 1, data_file);
    fclose(data_file);
  }

  dump_stats_csv(&mds, &confusions, &bt);
}
