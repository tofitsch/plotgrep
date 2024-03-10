#include "bitmap.h"

bitmap* bitmap_create(int w, int h) {

  bitmap *bm = (bitmap*) malloc(sizeof(bitmap));

  bm->w = w;
  bm->h = h;

  bm->data = (bool**) malloc(h * sizeof(bool*));

  return bm;

}
