#include "assert.h"
#include "errno.h"
#include "math.h"
#include "signal.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys/ioctl.h"
#include "sys/time.h"
#include "unistd.h"
#include <stdbool.h>
#include <stdint.h>

#define SL_IMPLEMENTATION
#include "sl.h"

#include "keys.h"
#include "stats.h"
#include "term_handler.h"
#include "text.h"
#include "wordlist.h"

#define LINE_SIZE_WORDS 20
#define POST_BUF_SZ 256

#define RED "\033[31m"
#define GRN "\033[34m"
#define RST "\033[0m"

typedef struct {
  char c;
  float time;
  float err_rate;
} ChrInfo;

static int CI_gt(const void *a, const void *b) {
  ChrInfo *ca = (ChrInfo *)a;
  ChrInfo *cb = (ChrInfo *)b;

  if (isnanf(ca->err_rate) && !isnanf(cb->err_rate)) {
    return INT32_MAX;
  }
  if (isnanf(cb->err_rate) && !isnanf(ca->err_rate)) {
    return INT32_MIN;
  }
  if (isnanf(cb->err_rate) && isnanf(ca->err_rate)) {
    return 0;
  }
  if (ca->err_rate == cb->err_rate) {
    return ca->time < cb->time;
  }
  return ca->err_rate < cb->err_rate;
}

static int CI_lt(const void *a, const void *b) {
  ChrInfo *ca = (ChrInfo *)a;
  ChrInfo *cb = (ChrInfo *)b;

  if (isnanf(ca->err_rate) && !isnanf(cb->err_rate)) {
    return INT32_MAX;
  }
  if (isnanf(cb->err_rate) && !isnanf(ca->err_rate)) {
    return INT32_MIN;
  }
  if (isnanf(cb->err_rate) && isnanf(ca->err_rate)) {
    return 0;
  }

  if (ca->err_rate == cb->err_rate) {
    return ca->time > cb->time;
  }

  return ca->err_rate > cb->err_rate;
}

static ChrInfo *CI_list_new(const MonoGramDataSummary *mds) {
  ChrInfo *chr_info = malloc(N_CHARS * sizeof(ChrInfo));
  for (long i = 0; i < N_CHARS; ++i) {
    chr_info[i].c = (char)(i + 32);
    chr_info[i].time = mds->times[i];
    chr_info[i].err_rate =
        (float)mds->n_misses[i] / (float)mds->n_occurrences[i];
  }
  return chr_info;
}

typedef struct {
  char bigram[2];
  float time;
  float err_rate;
} BigramInfo;

static int BI_gt(const void *a, const void *b) {
  BigramInfo *ba = (BigramInfo *)a;
  BigramInfo *bb = (BigramInfo *)b;

  if (isnanf(ba->err_rate) && !isnanf(bb->err_rate)) {
    return INT32_MAX;
  }
  if (!isnanf(ba->err_rate) && isnanf(bb->err_rate)) {
    return INT32_MIN;
  }
  if (isnanf(ba->err_rate) && isnanf(bb->err_rate)) {
    return 0;
  }

  if (ba->err_rate == bb->err_rate) {
    return ba->time < bb->time;
  }
  return ba->err_rate < bb->err_rate;
}

static int BI_lt(const void *a, const void *b) {
  BigramInfo *ba = (BigramInfo *)a;
  BigramInfo *bb = (BigramInfo *)b;

  if (isnanf(ba->err_rate) && !isnanf(bb->err_rate)) {
    return INT32_MAX;
  }
  if (!isnanf(ba->err_rate) && isnanf(bb->err_rate)) {
    return INT32_MIN;
  }
  if (isnanf(ba->err_rate) && isnanf(bb->err_rate)) {
    return 0;
  }

  if (ba->err_rate == bb->err_rate) {
    return ba->time > bb->time;
  }
  return ba->err_rate > bb->err_rate;
}

static BigramInfo *BI_list_new(const BigramTable *bt) {
  BigramInfo *bigram_info = malloc(N_CHARS * N_CHARS * sizeof(BigramInfo));
  for (long first = 0; first < N_CHARS; ++first) {
    for (long second = 0; second < N_CHARS; ++second) {
      const long idx = first * N_CHARS + second;
      bigram_info[idx].bigram[0] = keys[first];
      bigram_info[idx].bigram[1] = keys[second];
      bigram_info[idx].time = bt->avg_execution_time[first][second];
      bigram_info[idx].err_rate = (float)bt->n_misses[first][second] /
                                  (float)bt->n_occurrences[first][second];
    }
  }
  return bigram_info;
}

#define WORST_N 10

static WordList WL_update(const WordList *orig, const MonoGramDataSummary *mds,
                          const BigramTable *bt) {
  long *idcs = calloc((const unsigned long)orig->nwords, sizeof(long));
  long n_words = 0;

  ChrInfo *chr_info = CI_list_new(mds);
  qsort(chr_info, N_CHARS, sizeof(ChrInfo), &CI_gt);

  // BigramInfo *bigram_info = BI_list_new(bt);

  printf("1/4 of the words need to have a %c.\n", chr_info[0].c);
  while (n_words < orig->nwords / 4) {
    const long idx = rand() % orig->nwords;

    if (SL_contains(orig->words[idx], &chr_info[0].c, 1)) {
      idcs[n_words] = idx;
      ++n_words;
      continue;
    }
  }

  for (long i = 0; i < orig->nwords && n_words < 3 * orig->nwords / 4; ++i) {
    const long idx = rand() % orig->nwords;
    for (long n = 0; n < WORST_N; ++n) {
      if (SL_contains(orig->words[idx], &chr_info[n].c, 1)) {
        idcs[n_words] = idx;
        ++n_words;
        continue;
      }
    }
  }

  for (; n_words < orig->nwords; ++n_words) {
    idcs[n_words] = rand() % orig->nwords;
  }

  WordList ret = WL_sample(orig, idcs, orig->nwords);
  free(idcs);
  free(chr_info);
  return ret;
}

static bool validate_persist(MonoGramDataSummary *mds, ConfMatrix *cm,
                             BigramTable *bt) {
  (void) bt;
  (void) cm;
  return memcmp(mds, &(MonoGramDataSummary){0}, sizeof(MonoGramDataSummary)) ==
         0;
}

bool run = true;
bool canceled = false;

static void sigint_handler() {
  run = false;
  canceled = true;
}

#if 0
int main() {
  // create ConfMatrix if no file is found, else load data from file
  ConfMatrix *confusions = calloc(1, sizeof(ConfMatrix));
  BigramTable *bt = calloc(1, sizeof(BigramTable));
  MonoGramDataSummary *mds = calloc(1, sizeof(MonoGramDataSummary));

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
    fread(confusions, sizeof(ConfMatrix), 1, data_file);
    fread(mds, sizeof(MonoGramDataSummary), 1, data_file);
    fread(bt, sizeof(BigramTable), 1, data_file);
    fclose(data_file);
  }

  init_crc_table();
  srand(crc32((char *)confusions, sizeof(ConfMatrix)));

  WordList base = get_malloced_wordlist("./top3000en.txt");
  WordList w_list;
  if (memcmp(mds, &(MonoGramDataSummary){0}, sizeof(MonoGramDataSummary)) ==
      0) {
    WL_deepcopy(&base, &w_list);
  } else {
    w_list = WL_update(&base, mds, bt);
  }

  for (int i = 0; i < w_list.nwords; ++i) {
    printf(SL_FMT"\n", SL_FP(w_list.words[i]));
  }

  WL_free(w_list);
  WL_free(base);
}

#else
int main() {
  // set up interrupt handler
  struct sigaction sigterm_action = {0};
  sigterm_action.sa_handler = &sigint_handler;
  sigterm_action.sa_flags = 0;
  if (sigaction(SIGINT, &sigterm_action, NULL) != 0) {
    fprintf(stderr, "Error registering signal handler\nExiting...\n");
    exit(EXIT_FAILURE);
  }
  init_crc_table();
  char post_message[POST_BUF_SZ];
  memset(post_message, 0x0, POST_BUF_SZ);

  // create ConfMatrix if no file is found, else load data from file
  WordList base = get_malloced_wordlist("./top3000en.txt");
  while (!canceled) {
    run = true;
    ConfMatrix *confusions = calloc(1, sizeof(ConfMatrix));
    BigramTable *bt = calloc(1, sizeof(BigramTable));
    MonoGramDataSummary *mds = calloc(1, sizeof(MonoGramDataSummary));

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
      fread(confusions, sizeof(ConfMatrix), 1, data_file);
      fread(mds, sizeof(MonoGramDataSummary), 1, data_file);
      fread(bt, sizeof(BigramTable), 1, data_file);
      fclose(data_file);
    }

    srand(crc32((char *)confusions, sizeof(ConfMatrix)));

    WordList w_list;
    if (validate_persist(mds, confusions, bt)) {
      WL_deepcopy(&base, &w_list);
    } else {
      w_list = WL_update(&base, mds, bt);
    }

    int cur_line[LINE_SIZE_WORDS] = {0};
    for (int i = 0; i < LINE_SIZE_WORDS; ++i) {
      cur_line[i] = rand() % w_list.nwords;
    }

    // get terminal size
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    // suppress echoing
    init_term();

    Text text =
        T_create(&w_list, w.ws_row, w.ws_col, cur_line, LINE_SIZE_WORDS);

    clear();
    T_draw_all(text);

    TermPos term_pos = text.t_line_starts[0];

    goto_term_pos((TermPos){0});
    printf("Press [[space]] to start\n");
    printf("%s\n", post_message);

    goto_term_pos((TermPos){5, 0});

    BigramInfo *bi = BI_list_new(bt);
    ChrInfo *ci = CI_list_new(mds);
    qsort(bi, N_CHARS * N_CHARS, sizeof(BigramInfo), &BI_gt);
    qsort(ci, N_CHARS, sizeof(ChrInfo), &CI_gt);

    printf("Worst %i chars:\n", WORST_N);
    for (int i = 0; i < WORST_N; ++i) {
      printf("'%c'  ", ci[i].c);
    }

    printf("\nWorst %i bigrams:\n", WORST_N);
    for (int i = 0; i < WORST_N; ++i) {
      printf("'%s' ", bi[i].bigram);
    }

    qsort(bi, N_CHARS * N_CHARS, sizeof(BigramInfo), &BI_lt);
    qsort(ci, N_CHARS, sizeof(ChrInfo), &CI_lt);

    printf("\nBest %i chars:\n", WORST_N);
    for (int i = 0; i < WORST_N; ++i) {
      printf("'%c'  ", ci[i].c);
    }

    printf("\nBest %i bigrams:\n", WORST_N);
    for (int i = 0; i < WORST_N; ++i) {
      printf("'%s' ", bi[i].bigram);
    }

    while (run) {
      char c = getchar();
      if (c == ' ') {
        break;
      }
    }
    goto_term_pos((TermPos){0});
    printf("                        ");
    goto_term_pos(term_pos);

    struct timeval start, end;
    bool cur_char_wrong = false;
    gettimeofday(&start, NULL);
    while (run) {
      char c = getchar();
      // skip unprintable and control characters
      if (c < KC_SPC || c == KC_DEL) {
        continue;
      }
      gettimeofday(&end, NULL);
      const double time_ms = (double)(end.tv_sec - start.tv_sec) * 1000.0 +
                             (double)(end.tv_usec - start.tv_usec) / 1000.0;

      goto_term_pos((TermPos){1, 0});
      printf("Current Key Time: %.2f", time_ms);
      goto_term_pos(term_pos);

      if (!cur_char_wrong) {
        text.typedchars[text.cur_char] = c;
        text.time_to_type[text.cur_char] = time_ms;
      }

      if (c == text.chars[text.cur_char]) {
        goto_term_pos(term_pos);
        if (c == ' ') {
          c = '_';
        }
        fprintf(stdout, "%s%c" RST, (cur_char_wrong ? RED : GRN), c);

        if (!T_advance_char(&text, &term_pos)) {
          run = false;
        }

        cur_char_wrong = false;
        gettimeofday(&start, NULL);
      } else {
        cur_char_wrong = true;
        text.errors[text.cur_char] = true;
        ++text.n_errors;
      }
    }

    if (!canceled) {
      update_conf_matrix(confusions, &text);

      MDS_update(mds, &text);
      double total_time_ms = 0;
      for (int i = 0; i < text.n_chars; ++i) {
        total_time_ms += text.time_to_type[i];
      }

      BT_update(bt, &text);

      FILE *outfile = fopen(STORAGE_NAME, "w+");
      if (errno) {
        fprintf(stderr, "Error opening file %s for storage: %s\n", STORAGE_NAME,
                strerror(errno));
        exit(EXIT_FAILURE);
      }
      dump_stats_bin(outfile, mds, confusions, bt);
      fclose(outfile);

      const double cpm = (double)text.n_chars / total_time_ms * 60.0 * 1000.0;

      goto_term_pos((TermPos){1, 0});
      memset(post_message, 0x0, POST_BUF_SZ);
      snprintf(post_message, POST_BUF_SZ,
               GRN "Accuracy" RST ": %5.2f%% (%4i / %4i)\n" GRN
                   "Average Speed:" RST " %5.1f cpm / %5.1f wpm\n",
               (float)(text.n_chars - text.n_errors) / (float)text.n_chars,
               text.n_chars - text.n_errors, text.n_chars, cpm, cpm / 5.0f);
    }
    WL_free(w_list);
  }

  WL_free(base);
  // reset terminal
  goto_term_pos((TermPos){0});
  deinit_term();
  deinit_crc_table();

  return EXIT_SUCCESS;
}
#endif
