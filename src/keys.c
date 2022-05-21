#include "keys.h"

int char_idx(char c) { return c - KC_SPC; }

const char keys[N_CHARS] = " !\"#$%&'()*+,-./"
                           "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
                           "abcdefghijklmnopqrstuvwxyz{|}~";
