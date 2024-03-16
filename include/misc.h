#ifndef __misc_h_
#define __misc_h_

#include <stdbool.h>

void bits_to_hex(bool const [4], char*);
void hex_to_bits(bool [4], char const*);

bool is_in_rectangle(int, int, int, int, int, int);
bool rectangles_overlap(int, int, int, int, int, int, int, int);

#endif
