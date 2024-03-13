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

bitmap* bitmap_from_png(char *file_name, int threshold) {
  
  FILE * file = fopen(file_name, "r");

  unsigned char header[PNG_HEADER_BYTES];

  fread(header, 1, PNG_HEADER_BYTES, file);

  if (png_sig_cmp(header, 0, PNG_HEADER_BYTES)) {
    fclose(file);
    fprintf(stderr, "Error: %s is not a valid PNG file\n", file_name);
    return NULL;
  }

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr) {
    fclose(file);
    fprintf(stderr, "Error: png_create_read_struct failed\n");
    return NULL;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);

  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    fclose(file);
    fprintf(stderr, "Error: png_create_info_struct failed\n");
    return NULL;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(file);
    fprintf(stderr, "Error: Error during PNG read\n");
    return NULL;
  }

  png_init_io(png_ptr, file);
  png_set_sig_bytes(png_ptr, PNG_HEADER_BYTES);
  png_read_info(png_ptr, info_ptr);

  png_byte color_type = png_get_color_type(png_ptr, info_ptr);
  png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  int w = png_get_image_width(png_ptr, info_ptr);
  int h = png_get_image_height(png_ptr, info_ptr);

  if (color_type != PNG_COLOR_TYPE_GRAY || bit_depth != 8) {
    fclose(file);
    fprintf(stderr, "Error: Only 8-bit grayscale PNG images are supported\nUse `import -depth 8 -colorspace gray test.png`");
    return NULL;
  }

  bitmap *bm = bitmap_create(w, h);

  png_bytep row_data = (png_bytep) malloc(png_get_rowbytes(png_ptr, info_ptr));

  for (int y = 0; y < h; y++) {

    png_read_row(png_ptr, row_data, NULL);

    for (int x = 0; x < w; x++){
      bm->data[y][x] = row_data[x] > threshold ? 0 : 1;
    }

  }

  free(row_data);
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  fclose(file);

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
          if(rectangles_overlap(x, y, w_max_area, h_max_area, plots[i]->x, plots[i]->y, plots[i]->w, plots[i]->h))
            goto next;

        plots[(*n_plots)++] = bitmap_from_bitmap(bm, x, y, w_max_area, h_max_area);

      }

      next:;

    }
  }

}

bitmap* discrete_cosine_transform(bitmap* bm, int d){
  //2D DCT as defined in: https://doi.org/10.1117/12.853142

  int N = bm->w < bm->h ? bm->w : bm->h;

  bitmap *dct = bitmap_create(d, d);

  for (int u = 0; u < d ; ++u) {
    for (int v = 0; v < d; ++v) {

      float F = 0.;

      for (int x = 0; x < N ; ++x) {
        for (int y = 0; y < N ; ++y) {
          
          float f = (x < bm->w && y < bm->h) ? bm->data[y][x] : 0.;

          F += f * cos((2. * x + 1.) * u * M_PI / (2. * N)) * cos((2. * y + 1.) * v * M_PI / (2. * N));

        }
      }
      
      float alpha_u = 1. / sqrt((float) N);
      float alpha_v = 1. / sqrt((float) N);

      if(u > 0)
        alpha_u *= sqrt(2);

      if(v > 0)
        alpha_v *= sqrt(2);

      F *= alpha_u * alpha_v;

      dct->data[v][u] = F > 0. ? 0 : 1;

    }
  }

  return dct;

}

int bitmap_hamming_distance(bitmap *a, bitmap *b) {
    
    if(a->w != b->w || a->h != b->h){
      fprintf(stderr, "Error: in bitmap_hamming_distance: bitmaps must have same dimensions\n");
      return -1;
    }

    int dist = 0;

    for (int y = 0; y < a->h; ++y)
      for (int x = 0; x < a->w; ++x)
        if (a->data[y][x] != b->data[y][x])
          dist++;

    return dist;

}

char* bitmap_to_hex(bitmap* bm){
  
  char c;
  
//  for(int i = 0; i < bm->h * bm->w; i += 4) {
//
//    bits_to_hex(bm->data[i], &c);
//
//    printf("%c", c);
//
//  }

  for (int y = 0; y < bm->h; y++) {

    for (int x = 0; x < bm->w; x++)
      printf("%d", bm->data[y][x]);

    printf("\n");

  }

  for (int y = 0; y < bm->h; y++) {

    for (int x = 0; x < bm->w; x += 4) {

      bits_to_hex(&bm->data[y][x], &c);

      printf("%c", c);

    }

  }

  printf("\n");

}

bitmap* bitmap_from_hex(char* c, int w, int h){
  
  //TODO

}

void bitmap_destroy(bitmap* bm) {

  if (bm == NULL)
    return;

  for (int y = 0; y < bm->h; ++y)
    free(bm->data[y]);

  free(bm->data);
  free(bm);

}
