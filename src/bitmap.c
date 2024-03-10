#include "bitmap.h"

bitmap* bitmap_create(int w, int h) {

  bitmap *bm = (bitmap*) malloc(sizeof(bitmap));

  bm->w = w;
  bm->h = h;

  bm->data = (bool**) malloc(h * sizeof(bool*));

  for (int y = 0; y < h; ++y)
    bm->data[y] = (bool*) malloc(w * sizeof(bool));

  return bm;

}

bitmap* bitmap_from_pix(fz_pixmap *pix, int threshold) {
  
  bitmap *bm = bitmap_create(pix->w, pix->h);

  for (int y = 0; y < pix->h; ++y) {

    unsigned char *p = &pix->samples[y * pix->stride];

    for (int x = 0; x < pix->w; ++x) {

      bm->data[y][x] = *p > threshold ? 1 : 0;

      p++;

    }

  }

  return bm;

}

void bitmap_print(bitmap *bm, char * prefix){
  
  char suffix[] = ".ppm"; 

  char *file_name = (char *) calloc((strlen(prefix) + strlen(suffix) + 1), sizeof(char));

  strcat(file_name, prefix);
  strcat(file_name, suffix);

  printf("writing to %s\n", file_name);

  FILE * f = fopen(file_name, "w");

  fprintf(f, "P1\n");
  fprintf(f, "%d %d\n", bm->w, bm->h);

  for (int y = 0; y < bm->h; ++y) {
    for (int x = 0; x < bm->w; ++x) {

      if (x > 0)
        fprintf(f, " ");

      fprintf(f, "%d ", bm->data[y][x]);

    }

    fprintf(f, "\n");

  }

  fclose(f);

  free(file_name);

}

void bitmap_destroy(bitmap* bm) {

  if (bm == NULL)
    return;

  for (int y = 0; y < bm->h; ++y)
    free(bm->data[y]);

  free(bm->data);
  free(bm);

}
