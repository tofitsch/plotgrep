#ifndef __bitmap_h_
#define __bitmap_h_

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <png.h>
#include <mupdf/fitz.h>

#include "misc.h"

#define MIN_PLOT_AREA 1000
#define MAX_PLOT_ASPECT_RATIO 5
#define MAX_PLOTS_PER_PAGE 100
#define PNG_HEADER_BYTES 8

typedef struct {
  
  int x, y, w, h;

  bool **data;

} bm_BitMap;

bm_BitMap* bm_create(int, int);

bm_BitMap* bm_from_pix(fz_pixmap*, int);
bm_BitMap* bm_from_png(char*, int);
bm_BitMap* bm_from_bm(bm_BitMap*, int, int, int, int);

bm_BitMap* bm_discrete_cosine_transform(bm_BitMap*, int);

void bm_print(bm_BitMap*, char*);
void bm_find_plots(bm_BitMap*, bm_BitMap* [], int*, int);

char* bm_to_hex(bm_BitMap*);
bm_BitMap* bm_from_hex(char*, int, int);

int bm_hamming_distance(bm_BitMap*, bm_BitMap*);

void bm_destroy(bm_BitMap*);

#endif
