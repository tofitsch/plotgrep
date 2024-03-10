#include "bitmap.h"

bitmap* bitmap_create(int w, int h) {

  bitmap *bm = (bitmap*) malloc(sizeof(bitmap));

  bm->x = 0;
  bm->y = 0;

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

      bm->data[y][x] = *p > threshold ? 0 : 1;

      p++;

    }

  }

  return bm;

}

bitmap* bitmap_from_bitmap(bitmap *bm, int offset_x, int offset_y, int w, int h) {
  
  bitmap *new_bm = bitmap_create(w, h);

  new_bm->x = offset_x;
  new_bm->y = offset_y;

  for (int y = 0; y < h; ++y)
    for (int x = 0; x < w; ++x)
      new_bm->data[y][x] = bm->data[y + offset_y][x + offset_x];

  return new_bm;

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

void bitmap_find_plots(bitmap *bm, bitmap *plots[], int *n_plots){
  
  *n_plots = 0;

  int **ones_right = (int **) malloc(bm->h * sizeof(int *));
  int **ones_down = (int **) malloc(bm->h * sizeof(int *));

  for (int y = 0; y < bm->h; y++)
    ones_right[y] = (int *) malloc(bm->w * sizeof(int));

  for (int y = 0; y < bm->h; y++)
    ones_down[y] = (int *) malloc(bm->w * sizeof(int));

  for (int y = bm->h - 1; y >= 0; --y) {
    
    ones_right[y][bm->w - 1] = bm->data[y][bm->w - 1];
    
    for (int x = bm->w - 2; x >= 0; --x)
      ones_right[y][x] = bm->data[y][x] ? ones_right[y][x + 1] + 1 : 0;

  }

  for (int x = bm->w - 1; x >= 0; --x) {
    
    ones_down[bm->h - 1][x] = bm->data[bm->h - 1][x];
    
    for (int y = bm->h - 2; y >= 0; --y)
      ones_down[y][x] = bm->data[y][x] ? ones_down[y + 1][x] + 1 : 0;

  }

  for (int x = 0; x < bm->w; ++x) {
    for (int y = 0; y < bm->h; ++y) {
      
      if(ones_right[y][x] * ones_down[y][x] < MIN_PLOT_AREA)
        continue;
      
      int max_area = 0;
      int w_max_area, h_max_area;
      
      for (int X = ones_right[y][x] - 1; X>=0; --X) {
        for (int Y = ones_down[y][x + X] - 1; Y>=0; --Y) {

          if(ones_down[y][x] < Y)
            continue;

          if (ones_right[y + Y][x] >= X && X * Y > max_area && X * Y >= MIN_PLOT_AREA && X / Y < MAX_PLOT_ASPECT_RATIO && Y / X < MAX_PLOT_ASPECT_RATIO) {

            max_area = X * Y;
            
            w_max_area = X;
            h_max_area = Y;

          }
        }
      }

      if (max_area > 0) {
        
        for (int i = 0; i < *n_plots; ++i)
          if(bitmap_rectangles_overlap(x, y, w_max_area, h_max_area, plots[i]->x, plots[i]->y, plots[i]->w, plots[i]->h))
            goto next;

        plots[(*n_plots)++] = bitmap_from_bitmap(bm, x, y, w_max_area, h_max_area);

      }

      next:;

    }
  }

}

bool bitmap_rectangles_overlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {

    if (x1 >= x2 + w2 || x2 >= x1 + w1)
      return false;

    if (y1 >= y2 + h2 || y2 >= y1 + h1)
      return false;

    return true;

}

void bitmap_destroy(bitmap* bm) {

  if (bm == NULL)
    return;

  for (int y = 0; y < bm->h; ++y)
    free(bm->data[y]);

  free(bm->data);
  free(bm);

}
