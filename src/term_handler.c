#include "term_handler.h"

#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "termios.h"

void goto_term_pos(TermPos pos) { printf("\033[%d;%dH", pos.row, pos.col); }
void clear(void) { printf("\033[H\033[J"); }

void init_term(void) {
  clear();
  struct termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag &= (unsigned int)~ECHO;
  term.c_lflag &= (unsigned int)~ICANON;
  tcsetattr(STDIN_FILENO, 0, &term);
  setbuf(stdout, NULL);
}

void deinit_term(void) {
  struct termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag |= (unsigned int) ECHO;
  term.c_lflag |= (unsigned int) ICANON;
  tcsetattr(STDIN_FILENO, 0, &term);

  clear();
  goto_term_pos((TermPos) {0, 0});
}
