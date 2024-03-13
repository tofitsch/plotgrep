#include "misc.h"

void bits_to_hex(bool const bits[4], char *c) {

  *c = 0;

  for (int i = 0; i < 4; ++i)
    *c |= bits[i] << (3 - i);

  *c += *c > 9 ? 'A' - 10 : '0';

}

void hex_to_bits(bool bits[4], char const *c) {
  
  char const d = *c < 'A' ? *c - '0' : *c - 'A' + 10; 

  for (int i = 0; i < 4; ++i)
    bits[i] = (d >> (3 - i)) & 1;

}

bool rectangles_overlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {

  if (x1 >= x2 + w2 || x2 >= x1 + w1)
    return false;

  if (y1 >= y2 + h2 || y2 >= y1 + h1)
    return false;

  return true;

}

