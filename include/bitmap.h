#ifndef __bitmap_h_
#define __bitmap_h_

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <png.h>
#include <mupdf/fitz.h>

#include "misc.h"

#define MAX_RECIP_PLOT_AREA_OF_TOTAL_PDF 100
#define MAX_RECIP_PLOT_AREA_OF_TOTAL_PNG 1
#define MAX_PLOT_ASPECT_RATIO 3
#define MAX_PLOTS_PER_PAGE 16
#define PNG_HEADER_BYTES 8

typedef struct {
  
  int x, y, w, h;

  bool **data;

} bm_BitMap;

bm_BitMap* bm_create(int, int);

int bm_by_area(const void*, const void*);

bm_BitMap* bm_from_pdf(fz_pixmap*);
bm_BitMap* bm_from_png(png_bytep [], int, int);

bm_BitMap* bm_from_hex(char*, int, int);
char* bm_to_hex(bm_BitMap*);

bm_BitMap* bm_crop_from_pdf(fz_pixmap*, int, int, int, int);
bm_BitMap* bm_crop_from_png(png_bytep [], int, int, int, int);

void bm_find_plots(bm_BitMap*, bm_BitMap* [], int*, int, fz_pixmap*, png_bytep []);

bm_BitMap* bm_discrete_cosine_transform(bm_BitMap*, int);

int bm_hamming_distance(bm_BitMap*, bm_BitMap*);

void bm_print(bm_BitMap*, char*);

void bm_destroy(bm_BitMap*);

#endif
