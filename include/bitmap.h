#ifndef __bitmap_h_
#define __bitmap_h_

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <mupdf/fitz.h>

typedef struct {
  
  int w, h;

  bool **data;

} bitmap;

bitmap* bitmap_create(int, int);
bitmap* bitmap_from_pix(fz_pixmap*, int);

void bitmap_print(bitmap*, char*);

void bitmap_destroy(bitmap*);

#endif
