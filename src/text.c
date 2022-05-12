#include "text.h"

#include "assert.h"
#include "memory.h"
#include "stdlib.h"
#include "term_handler.h"

#define VERT_BOUND_LINES 5
#define HORZ_BOUND_CHARS 10

#define check_pos(wl, pos)                                                     \
  assert(pos.word < wl.nwords && pos.word >= 0);                               \
  assert(pos.chr_idx < wl.words[pos.word].len && pos.chr_idx >= 0)

Text T_create(WordList *w_list, int term_rows, int term_cols, int *indices,
              int n_words) {
  assert(n_words > 0);

  // determine number of chars
  int n_chars = 0;
  for (int i = 0; i < n_words; ++i) {
    n_chars += w_list->words[indices[i]].len;
  }
  // account for spaces
  n_chars += n_words - 1;

  assert(n_chars > 0);
  char *allchars = calloc((unsigned long)n_chars, sizeof(char));

  // initialise these two as oversize and then realloc
  int *line_sizes = calloc((unsigned long)n_words, sizeof(int));
  int *line_sizes_chars = calloc((unsigned long)n_words, sizeof(int));

  int cur_char_idx = 0;
  int cur_line = 0;

  for (int i = 0; i < n_words; ++i) {
    const SL *cur_word = &w_list->words[indices[i]];
    assert(cur_word->len > 0);

    if (line_sizes_chars[cur_line] + cur_word->len + 1 >=
        term_cols - 2 * HORZ_BOUND_CHARS) {
      // add last space in line
      allchars[cur_char_idx] = ' ';
      ++cur_char_idx;
      ++line_sizes_chars[cur_line];

      ++cur_line;
    }

    if (line_sizes_chars[cur_line] != 0) {
      // spaces
      allchars[cur_char_idx] = ' ';
      ++cur_char_idx;
      ++line_sizes_chars[cur_line];
    }

    memcpy(&allchars[cur_char_idx], cur_word->start,
           (unsigned long)cur_word->len);

    ++line_sizes[cur_line];
    cur_char_idx += cur_word->len;
    line_sizes_chars[cur_line] += cur_word->len;
  }

  const int n_lines = cur_line + 1;
  assert(n_lines > 0);
  int *line_starts = calloc((unsigned long)n_lines, sizeof(int));
  line_sizes = realloc(line_sizes, (unsigned long)n_lines * sizeof(int));

  for (int line = 1; line < n_lines; ++line) {
    line_starts[line] = line_starts[line - 1] + line_sizes[line - 1];
  }

  TermPos *t_line_starts = malloc((unsigned long)n_lines * sizeof(TermPos));
  for (int line = 0; line < n_lines; ++line) {
    t_line_starts[line] =
        (TermPos){.col = (term_cols - line_sizes_chars[line]) / 2,
                  .row = line + (term_rows - n_lines) / 2};
  }

  printf("\n");

  int *word_idcs = malloc((unsigned long)n_words * sizeof(int));
  memcpy(word_idcs, indices, (unsigned long)n_words * sizeof(int));

  Text ret = {
      .t_line_starts = t_line_starts,

      .line_sizes = line_sizes,
      .line_sizes_chars = line_sizes_chars,
      .line_starts = line_starts,

      .chars = allchars,
      .n_chars = n_chars,
      .n_words = n_words,
      .n_lines = n_lines,
      .word_idcs = word_idcs,
      .w_list = w_list,

      .cur_char = 0,
      .cur_line_char = 0,
      .time_to_type = malloc((unsigned long)n_chars * sizeof(double)),
      .typedchars = malloc((unsigned long)n_chars * sizeof(char)),
      .errors = malloc((unsigned long)n_chars * sizeof(bool)),
  };

  return ret;
}

void T_draw_all(Text t) {
  for (int line = 0; line < t.n_lines; ++line) {
    goto_term_pos(t.t_line_starts[line]);

    for (int l_word_idx = 0; l_word_idx < t.line_sizes[line]; ++l_word_idx) {
      const int word_idx = t.word_idcs[l_word_idx + t.line_starts[line]];
      printf(SL_FMT " ", SL_FP(t.w_list->words[word_idx]));
    }
  }
}

bool T_advance_char(Text *t, TermPos *term_pos) {
  ++t->cur_char;
  ++t->cur_line_char;

  const int line = t->pos.line;
  if (t->cur_line_char >= t->line_sizes_chars[line]) {
    if (line == t->n_lines - 1) {
      return false;
    }

    ++t->pos.line;
    t->cur_line_char = 0;
    const TermPos next_line_start = t->t_line_starts[line + 1];
    term_pos->row = next_line_start.row;
    term_pos->col = next_line_start.col;
  } else {
    ++term_pos->col;
  }

  return true;
}
