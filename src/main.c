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
#include "database.h"

#define DEBUG false
#define PDF_ZOOM 2
#define THRESHOLD 200
#define TMP_FILE "tmp.png"
#define DCT_DIMENSION 16 //TODO: must be divisible by 4 (for hex encoding)
#define MAX_DB_ENTRIES 128

bm_BitMap* get_plot_from_screen_grab(){
  
  int n_plots = 0;
  bm_BitMap *plots[MAX_PLOTS_PER_PAGE];

  printf("select plot to search\n");

  char command[] = "import -depth 8 -colorspace gray ";

  char *full_command = (char *) calloc((strlen(command) + strlen(TMP_FILE) + 1), sizeof(char));

  strcat(full_command, command);
  strcat(full_command, TMP_FILE);

  system(full_command); //TODO: remove system call

  bm_BitMap *bm = bm_from_png(TMP_FILE, THRESHOLD);

  remove(TMP_FILE); //TODO: remove system call
  
  bm_find_plots(bm, plots, &n_plots); //TODO: use all plots found rather than just return first

  if(DEBUG)
    bm_print(plots[0], "pngtest_plot");

  if(DEBUG)
    bm_print(bm, "pngtest");

  for(int i = 1; i < n_plots; ++i) //TODO: when doing loop over all plots, note the start from 1
    bm_destroy(plots[i]);

  bm_destroy(bm);

  return plots[0];

}

int main(int argc, char **argv) {
  
  int n_db = 0;
  db_Entry db[MAX_DB_ENTRIES];
  
  bm_BitMap *plot_screen_grab = get_plot_from_screen_grab();

  bm_BitMap *dct_screen_grab = discrete_cosine_transform(plot_screen_grab, DCT_DIMENSION);

  char *hex = bm_to_hex(dct_screen_grab);
  printf("%s\n", hex);
  free(hex);
//  bm_BitMap *test = bm_from_hex(hex, DCT_DIMENSION, DCT_DIMENSION); //XXX
//  for (int y = 0; y < test->h; y++) {
//    for (int x = 0; x < test->w; ++x) {
//       
//       printf("%d", test->data[y][x]);
//    }
//
//    printf("\n");
//
//  }
//
//  bm_destroy(test); //XXX
  bm_BitMap *plots[MAX_PLOTS_PER_PAGE];
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

      bm_BitMap *bm = bm_from_pix(pix, THRESHOLD);

      bm_find_plots(bm, plots, &n_plots);

      for(int i = 0; i < n_plots; ++i){

        char *plot_name = (char *) calloc(strlen("doc_XXXX_page_XXXX_plot_XXXX"), sizeof(char));

        sprintf(plot_name, "doc_%03d_page_%03d_plot_%03d", d + 1, page_number + 1, i + 1);

        bm_BitMap *dct = discrete_cosine_transform(plots[i], DCT_DIMENSION);

        char *hex = bm_to_hex(dct); //XXX
        printf("%s\n", hex);
         
        int dist = bm_hamming_distance(dct, dct_screen_grab);

        if(DEBUG)
          printf("dist: %d\n", dist);

        bm_destroy(dct);

        if(DEBUG)
          bm_print(plots[i], plot_name);

        db[n_db].dist = dist;
        db[n_db].hex = hex;
        db[n_db].name = plot_name;

        n_db++;

      }

      char page_name[32];

      sprintf(page_name, "doc_%03d_page_%03d", d + 1, page_number + 1);

      if(DEBUG)
        bm_print(bm, page_name);

      /* Clean up. */
      bm_destroy(bm);

      for(int i = 0; i < n_plots; ++i)
        bm_destroy(plots[i]);

      fz_drop_pixmap(ctx, pix);

    }

    fz_drop_document(ctx, doc);

    fz_drop_context(ctx);

  }

  bm_destroy(plot_screen_grab);
  bm_destroy(dct_screen_grab);

  qsort(db, n_db, sizeof(db_Entry), db_by_dist);

  for(int i = 0; i < n_db ; ++i)
    printf("%04d %s %s\n", db[i].dist, db[i].hex, db[i].name);

  for(int i = 0; i < n_db ; ++i)
    db_destroy_entry(&db[i]);

  return EXIT_SUCCESS;

}
