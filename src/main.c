// https://codeberg.org/caps-universe/libcapsraster/src/tag/libcapsraster-1.0.1/test/test-simple-mupdf.c
// https://mupdf.com/docs/examples/example.c
// sudo apt-get install libmupdf-dev ljbig2dec 
// sudo ln -s /usr/lib/x86_64-linux-gnu/libopenjp2.so.2.3.1 /usr/lib/x86_64-linux-gnu/libopenjp2.so
// sudo ln -s /usr/lib/x86_64-linux-gnu/libjbig2dec.so.0 /usr/lib/x86_64-linux-gnu/libjbig2dec.so
// gcc -o x extract_images.c /usr/lib/libmupdf.a /usr/lib/libmupdf-third.a -lmupdf -lm -lmupdf-third -lfreetype -lharfbuzz -ljpeg -lz -L/usr/lib/x86_64-linux-gnu -lopenjp2 -ljbig2dec
// ./x test.pdf

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <mupdf/fitz.h>

#include "bitmap.h"

#define DEBUG false
#define PDF_ZOOM 2
#define THRESHOLD 200
#define TMP_FILE "tmp.png"
#define DCT_DIMENSION 16
#define MAX_DB_ENTRIES 128

//TODO: refactor
typedef struct {

  int doc, page, plot, dist;

} dbentry;

int by_dist(const void * a, const void * b) {

  const dbentry * A = (const dbentry *) a;
  const dbentry * B = (const dbentry *) b;

  return A->dist < B->dist;

}

bitmap* get_plot_from_screen_grab(){
  
  int n_plots = 0;
  bitmap *plots[MAX_PLOTS_PER_PAGE];

  printf("select plot to search\n");

  char command[] = "import -depth 8 -colorspace gray ";

  char *full_command = (char *) calloc((strlen(command) + strlen(TMP_FILE) + 1), sizeof(char));

  strcat(full_command, command);
  strcat(full_command, TMP_FILE);

  system(full_command); //TODO: remove system call

  bitmap *bm = bitmap_from_png(TMP_FILE, THRESHOLD);

  remove(TMP_FILE); //TODO: remove system call
  
  bitmap_find_plots(bm, plots, &n_plots); //TODO: use all plots found rather than just return first

  if(DEBUG)
    bitmap_print(plots[0], "pngtest_plot");

  if(DEBUG)
    bitmap_print(bm, "pngtest");

  for(int i = 1; i < n_plots; ++i) //TODO: when doing loop over all plots, note the start from 1
    bitmap_destroy(plots[i]);

  bitmap_destroy(bm);

  return plots[0];

}

int main(int argc, char **argv) {
  
  int n_db = 0;
  dbentry db[MAX_DB_ENTRIES];
  
  bitmap *plot_screen_grab = get_plot_from_screen_grab();

  bitmap *dct_screen_grab = discrete_cosine_transform(plot_screen_grab, DCT_DIMENSION);

  char plot_name[32];
  char page_name[32];

  bitmap *plots[MAX_PLOTS_PER_PAGE];
  int n_plots = 0;

  int page_count;

  fz_context *ctx;
  fz_document *doc;
  fz_pixmap *pix;
  fz_matrix mtx;

  if (argc < 2) {
    fprintf(stderr, "usage: plotgrep <input.pdf>\n");
    return EXIT_FAILURE;
  }

  for(int d = 0; d < argc - 1; ++d){

    /* Create a context to hold the exception stack and various caches. */
    ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
    if (!ctx)
    {
      fprintf(stderr, "cannot create mupdf context\n");
      return EXIT_FAILURE;
    }

    /* Register the default file types to handle. */
    fz_try(ctx)
      fz_register_document_handlers(ctx);
    fz_catch(ctx)
    {
      fprintf(stderr, "cannot register document handlers: %s\n", fz_caught_message(ctx));
      fz_drop_context(ctx);
      return EXIT_FAILURE;
    }
    
    /* Open the document. */
    fz_try(ctx)
      doc = fz_open_document(ctx, argv[1 + d]);
    fz_catch(ctx)
    {
      fprintf(stderr, "cannot open document: %s\n", fz_caught_message(ctx));
      fz_drop_context(ctx);
      return EXIT_FAILURE;
    }

    printf("reading plots from %s\n", argv[1 + d]);

    /* Count the number of pages. */
    fz_try(ctx)
      page_count = fz_count_pages(ctx, doc);
    fz_catch(ctx)
    {
      fprintf(stderr, "cannot count number of pages: %s\n", fz_caught_message(ctx));
      fz_drop_document(ctx, doc);
      fz_drop_context(ctx);
      return EXIT_FAILURE;
    }

    mtx = fz_scale(PDF_ZOOM, PDF_ZOOM);

    for (int page_number = 0; page_number < page_count; ++page_number) {

      n_plots = 0;

      /* Render page to an RGB pixmap. */
      fz_try(ctx)
        pix = fz_new_pixmap_from_page_number(ctx, doc, page_number, mtx, fz_device_gray(ctx), 0);
      fz_catch(ctx)
      {
        fprintf(stderr, "cannot render page: %s\n", fz_caught_message(ctx));
        fz_drop_document(ctx, doc);
        fz_drop_context(ctx);
        return EXIT_FAILURE;
      }

      bitmap *bm = bitmap_from_pix(pix, THRESHOLD);

      bitmap_find_plots(bm, plots, &n_plots);

      for(int i = 0; i < n_plots; ++i){

        sprintf(plot_name, "doc_%03d_page_%03d_plot_%03d", d + 1, page_number + 1, i + 1);

        bitmap *dct = discrete_cosine_transform(plots[i], DCT_DIMENSION);
         
        int dist = bitmap_hamming_distance(dct, dct_screen_grab);

        if(DEBUG)
          printf("dist: %d\n", dist);

        bitmap_destroy(dct);

        if(DEBUG)
          bitmap_print(plots[i], plot_name);

        db[n_db].doc = d + 1;
        db[n_db].page = page_number + 1;
        db[n_db].plot = i + 1;
        db[n_db].dist = dist;
        n_db++;

      }

      sprintf(page_name, "doc_%03d_page_%03d", d + 1, page_number + 1);

      if(DEBUG)
        bitmap_print(bm, page_name);

      /* Clean up. */
      bitmap_destroy(bm);

      for(int i = 0; i < n_plots; ++i)
        bitmap_destroy(plots[i]);

      fz_drop_pixmap(ctx, pix);

    }

    fz_drop_document(ctx, doc);

    fz_drop_context(ctx);

  }

  bitmap_destroy(plot_screen_grab);
  bitmap_destroy(dct_screen_grab);

  qsort(db, n_db, sizeof(dbentry), by_dist);

  for(int i = 0; i < n_db ; ++i)
    printf("%d\t %s page %d plot %d\n", db[i].dist, argv[db[i].doc], db[i].page, db[i].plot);

  return EXIT_SUCCESS;

}
