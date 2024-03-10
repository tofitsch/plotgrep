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

#define ZOOM 2
#define THRESHOLD 200

int main(int argc, char **argv) {

  int page_count;

  fz_context *ctx;
  fz_document *doc;
  fz_pixmap *pix;
  fz_matrix mtx;

  if (argc != 2) {
    fprintf(stderr, "usage: plotgrep <input.pdf>\n");
    return EXIT_FAILURE;
  }

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
    doc = fz_open_document(ctx, argv[1]);
  fz_catch(ctx)
  {
    fprintf(stderr, "cannot open document: %s\n", fz_caught_message(ctx));
    fz_drop_context(ctx);
    return EXIT_FAILURE;
  }

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

  mtx = fz_scale(ZOOM, ZOOM);

  char plot_name[32];
  char page_name[32];

  for (int page_number = 1; page_number < page_count; ++page_number) {

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

    bitmap *plots[MAX_PLOTS_PER_PAGE];
    int n_plots = 0;

    bitmap_find_plots(bm, plots, &n_plots);

    for(int i = 0; i < n_plots; ++i){

      sprintf(plot_name, "page_%03d_plot_%03d", page_number, i);

      bitmap_print(plots[i], plot_name);

    }

    sprintf(page_name, "page_%03d", page_number);

    bitmap_print(bm, page_name);

    /* Clean up. */
    bitmap_destroy(bm);

    for(int i = 0; i < n_plots; ++i)
      bitmap_destroy(plots[i]);

    fz_drop_pixmap(ctx, pix);

  }

  fz_drop_document(ctx, doc);
  fz_drop_context(ctx);
  return EXIT_SUCCESS;

}
