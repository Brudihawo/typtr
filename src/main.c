#include "assert.h"
#include "signal.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys/ioctl.h"
#include "termios.h"
#include "unistd.h"
#include <stdbool.h>

#define SL_IMPLEMENTATION
#include "sl.h"

#include "wordlist.h"

#define LINE_SIZE_WORDS 5

#define RED "\033[31m"
#define GRN "\033[34m"
#define RST "\033[0m"

void get_random_words(const WordList w_list, long *word_idcs) {
  for (long i = 0; i < LINE_SIZE_WORDS; ++i) {
    word_idcs[i] = rand() % w_list.nwords;
  }
}

int get_line_len(WordList w_list, long *cur_line) {
  int ret = 0;
  for (long i = 0; i < LINE_SIZE_WORDS; ++i) {
    assert(cur_line[i] < w_list.nwords && cur_line[i] > 0);
    ret += w_list.words[cur_line[i]].len;
  }
  return ret;
}

void goto_term_pos(int x, int y) { printf("\033[%d;%dH", x, y); }
void clear(void) { printf("\033[H\033[J"); }

bool run = true;

static void sigint_handler(int sig) { run = false; }

void init_term() {
  struct termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag &= ~ECHO;
  term.c_lflag &= ~ICANON;
  tcsetattr(STDIN_FILENO, 0, &term);
  setbuf(stdout, NULL);
}

void deinit_term() {
  goto_term_pos(0, 0);
  struct termios term;
  tcgetattr(STDIN_FILENO, &term);
  clear();
  term.c_lflag |= ECHO;
  term.c_lflag |= ICANON;
  tcsetattr(STDIN_FILENO, 0, &term);
}

typedef struct {
  int row, col;
} TermPos;

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
  long cur_line[LINE_SIZE_WORDS] = {0};

  // get terminal size
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  // clear terminal
  for (long row = 0; row < w.ws_row; ++row) {
    printf("%*c\n", w.ws_col, ' ');
  }

  // suppress echoing
  init_term();

  get_random_words(w_list, cur_line);

  // TODO: proper handling of the current line size (with wrapping etc)
  const long line_len = get_line_len(w_list, cur_line);
  assert(line_len < w.ws_col);

  long row = w.ws_row / 2;
  goto_term_pos(row, (int)(w.ws_col - line_len) / 2);

  for (int i = 0; i < LINE_SIZE_WORDS; ++i) {
    printf(SL_FMT " ", SL_FP(w_list.words[cur_line[i]]));
  }

  int cur_word_idx = 0;
  int cur_char_idx = 0;
  TermPos pos = {row, (w.ws_col - line_len) / 2};

  goto_term_pos(pos.row, pos.col);
  bool cur_char_wrong = false;
  while (run) {
    const SL *cur_word = &w_list.words[cur_line[cur_word_idx]];
    char c = getchar();

    if (cur_char_idx == cur_word->len) {
      if (c == ' ') {
        goto_term_pos(pos.row, pos.col);
        fprintf(stdout, "%s%c" RST, (cur_char_wrong ? RED : GRN),
                cur_char_wrong ? '_' : c);
        if (cur_word_idx == LINE_SIZE_WORDS - 1) {
          run = false;
        } else {
          cur_char_idx = 0;
          ++cur_word_idx;
          ++pos.col;
          cur_char_wrong = false;
        }
      } else {
        cur_char_wrong = true;
      }
    } else {
      if (c == SL_at(*cur_word, cur_char_idx)) {
        goto_term_pos(pos.row, pos.col);
        fprintf(stdout, "%s%c" RST, (cur_char_wrong ? RED : GRN), c);
        cur_char_wrong = false;
        ++cur_char_idx;
        ++pos.col;
      } else {
        cur_char_wrong = true;
      }
    }
  }

  // reset terminal
  deinit_term();

  WL_free(w_list);
  return EXIT_SUCCESS;
}
