#include "assert.h"
#include "errno.h"
#include "signal.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys/ioctl.h"
#include "sys/time.h"
#include "unistd.h"
#include <stdbool.h>

#define SL_IMPLEMENTATION
#include "sl.h"

#include "stats.h"
#include "term_handler.h"
#include "text.h"
#include "wordlist.h"

#define LINE_SIZE_WORDS 20

#define RED "\033[31m"
#define GRN "\033[34m"
#define RST "\033[0m"

volatile bool run = true;
volatile bool canceled = false;

static void sigint_handler() {
  run = false;
  canceled = true;
}

int main() {
  // set up interrupt handler
  struct sigaction sigterm_action = {0};
  sigterm_action.sa_handler = &sigint_handler;
  sigterm_action.sa_flags = 0;
  if (sigaction(SIGINT, &sigterm_action, NULL) != 0) {
    fprintf(stderr, "Error registering signal handler\nExiting...\n");
    exit(EXIT_FAILURE);
  }
 
  // create ConfMatrix if no file is found, else load data from file
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

  init_crc_table();
  srand(crc32((char *)&confusions, sizeof(confusions)));

  WordList w_list = get_malloced_wordlist("./top1000en.txt");
  int cur_line[LINE_SIZE_WORDS] = {0};
  for (int i = 0; i < LINE_SIZE_WORDS; ++i) {
    cur_line[i] = rand() % w_list.nwords;
  }

  // get terminal size
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  // suppress echoing
  init_term();

  Text text = T_create(&w_list, w.ws_row, w.ws_col, cur_line, LINE_SIZE_WORDS);

  clear();
  T_draw_all(text);

  TermPos term_pos = text.t_line_starts[0];

  goto_term_pos((TermPos){0});
  printf("Press [[space]] to start");

  while (true) {
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
    if (c < 32 || c == 127) {
      continue;
    }
    gettimeofday(&end, NULL);
    const double time_ms = (double)(end.tv_sec - start.tv_sec) * 1000.0 +
                           (double)(end.tv_usec - start.tv_usec) / 1000.0;

    goto_term_pos((TermPos){0});
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

  // reset terminal
  goto_term_pos((TermPos){0});
  deinit_term();

  if (!canceled) {
    update_conf_matrix(&confusions, &text);

    MDS_update(&mds, &text);
    double total_time_ms = 0;
    for (int i = 0; i < text.n_chars; ++i) {
      total_time_ms += text.time_to_type[i];
    }

    BT_update(&bt, &text);

    FILE *outfile = fopen(STORAGE_NAME, "w+");
    if (errno) {
      fprintf(stderr, "Error opening file %s for storage: %s\n", STORAGE_NAME,
              strerror(errno));
      exit(EXIT_FAILURE);
    }
    dump_stats_bin(outfile, &mds, &confusions, &bt);
    fclose(outfile);

    WL_free(w_list);
    deinit_crc_table();
    const double cpm = (double)text.n_chars / total_time_ms * 60.0 * 1000.0;

    printf(GRN "Accuracy" RST ": %f%% (%i / %i)\n" GRN "Average Speed:" RST
               " %f cpm / %f wpm\n",
           (float)(text.n_chars - text.n_errors) / (float)text.n_chars,
           text.n_chars - text.n_errors, text.n_chars, cpm,
           cpm / 5.0f);
  }

  WL_free(w_list);
  deinit_crc_table();
  return EXIT_SUCCESS;
}
