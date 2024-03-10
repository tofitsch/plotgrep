#ifndef __bitmap_h_
#define __bitmap_h_

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
  
  int w, h;

  bool **data;

} bitmap;

bitmap* bitmap_create(int, int);

#endif
