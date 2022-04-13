#ifndef TERM_HANDLER_H
#define TERM_HANDLER_H

typedef struct {
  int row, col;
} TermPos;

void goto_term_pos(TermPos pos);

void clear(void);

void init_term(void);

void deinit_term(void);

#endif // TERM_HANDLER_H
