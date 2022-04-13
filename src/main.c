#include "assert.h"
#include "signal.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys/ioctl.h"
#include "sys/time.h"
#include "unistd.h"
#include <stdbool.h>

#define SL_IMPLEMENTATION
#include "sl.h"

#include "term_handler.h"
#include "text.h"
#include "wordlist.h"

#define LINE_SIZE_WORDS 5

#define RED "\033[31m"
#define GRN "\033[34m"
#define RST "\033[0m"

bool run = true;

static void sigint_handler() { run = false; }

#define N_CHARS 27
const char keys[N_CHARS] = {"abcdefghijklmnopqrstuvwxyz "};

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
  float execution_time[N_CHARS][N_CHARS];
  long n_occurrences[N_CHARS][N_CHARS];
  long n_misses[N_CHARS][N_CHARS];
} BigramTable;

static void update_conf_matrix(ConfMatrix *mat, Text *t) {
  mat->n_hits = t->n_chars;
  for (int i = 0; i < t->n_chars; ++i) {
    const int correct_idx = t->chars[i] == ' ' ? 26 : t->chars[i] - 97;
    const int actual_idx = t->typedchars[i] == ' ' ? 26 : t->typedchars[i] - 97;
    ++mat->matrix[correct_idx][actual_idx];
  }
}

static void print_conf_matrix(ConfMatrix *mat) {
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

static MonoGramDataSummary build_monogram_data(Text *t) {
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

static void print_mds(MonoGramDataSummary* mds) {
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

int main() {
  // set up interrupt handler
  struct sigaction sigterm_action = {0};
  sigterm_action.sa_handler = &sigint_handler;
  sigterm_action.sa_flags = 0;
  if (sigaction(SIGINT, &sigterm_action, NULL) != 0) {
    fprintf(stderr, "Error registering signal handler\nExiting...\n");
    exit(EXIT_FAILURE);
  }

  WordList w_list = get_malloced_wordlist("./top1000en.txt");
  int cur_line[LINE_SIZE_WORDS] = {0};
  for (int i = 0; i < LINE_SIZE_WORDS; ++i) {
    cur_line[i] = rand() % 1000;
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
  while (run) {
    gettimeofday(&start, NULL);
    char c = getchar();
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
    } else {
      cur_char_wrong = true;
      text.errors[text.cur_char] = true;
    }
  }

  // reset terminal
  goto_term_pos((TermPos){0});
  deinit_term();

  ConfMatrix confusions = {0};
  update_conf_matrix(&confusions, &text);

  MonoGramDataSummary mds = build_monogram_data(&text);
  print_mds(&mds);

  WL_free(w_list);
  return EXIT_SUCCESS;
}
