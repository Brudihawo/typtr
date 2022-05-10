#ifndef TEXT_H
#define TEXT_H

#include "term_handler.h"
#include "wordlist.h"

typedef struct {
  int line;
  int word;
  int chr_idx;
} TextPos;

typedef struct {
  TextPos pos;

  int cur_char;
  int cur_line_char;

  const int n_chars;
  const char* chars;

  const int n_lines;
  const int* line_sizes_chars;
  const int* nwords_l;

  const TermPos* t_line_starts;

  const WordList *w_list;
  const int *word_idcs;
  const int n_words;

  const int *line_starts;
  const int *line_sizes;

  bool *errors;
  char *typedchars;
  double *time_to_type;
} Text;

Text T_create(WordList *w_list, int term_rows, int term_cols, int *indices,
              int n_words);

void T_draw_all(Text t);

bool T_advance_char(Text *t, TermPos* term_pos);
#endif // TEXT_H
