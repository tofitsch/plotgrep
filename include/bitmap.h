#ifndef __bitmap_h_
#define __bitmap_h_

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <png.h>
#include <mupdf/fitz.h>

#define MIN_PLOT_AREA 1000
#define MAX_PLOT_ASPECT_RATIO 5
#define MAX_PLOTS_PER_PAGE 100
#define PNG_HEADER_BYTES 8

typedef struct {
  
  int x, y, w, h;

  bool **data;

} bitmap;

bitmap* bitmap_create(int, int);

bitmap* bitmap_from_pix(fz_pixmap*, int);
bitmap* bitmap_from_png(char*, int);
bitmap* bitmap_from_bitmap(bitmap*, int, int, int, int);

bitmap* discrete_cosine_transform(bitmap*, int);

void bitmap_print(bitmap*, char*);
void bitmap_find_plots(bitmap*, bitmap* [], int*);

bool bitmap_rectangles_overlap(int, int, int, int, int, int, int, int);

int bitmap_hamming_distance(bitmap*, bitmap*);

void bitmap_destroy(bitmap*);

#endif
