#include "io.h"

bm_BitMap* io_get_plot_from_screen_grab(int dct_dimension, int threshold) {

  printf("select plot to search\n");

  char command[] = "import -depth 8 -colorspace gray ";

  char *full_command = (char *) calloc((strlen(command) + strlen(TMP_FILE) + 1), sizeof(char));

  strcat(full_command, command);
  strcat(full_command, TMP_FILE);

  system(full_command); //TODO: remove system call

  free(full_command);

  bm_BitMap *bm = bm_from_png(TMP_FILE, threshold);

  #ifdef DEBUG
  bm_print(bm, "pngtest");
  #endif

  remove(TMP_FILE); //TODO: remove system call
  
  int n_plots = 0;
  bm_BitMap *plots[1];

  bm_find_plots(bm, plots, &n_plots, 1);

  bm_destroy(bm);

  if (n_plots == 0) {
    fprintf(stderr, "cannot find plot in screen grab\n");
    exit(EXIT_FAILURE);
  }

  #ifdef DEBUG
  bm_print(plots[0], "pngtest_plot");
  #endif

  bm_BitMap *dct_screen_grab = bm_discrete_cosine_transform(plots[0], dct_dimension);

  bm_destroy(plots[0]);

  return dct_screen_grab;

}

void io_add_plots_from_pdf(char *file_name, db_Entry db[], int *n_db, int dct_dimension, int threshold, int pdf_zoom) {

  fz_context *ctx;
  fz_document *doc;
  fz_pixmap *pix;
  fz_matrix mtx;

  ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
  if (!ctx) {
    fprintf(stderr, "WARNING: cannot create mupdf context.\nskipping %s\n", file_name);
    return;
  }

  /* Register the default file types to handle. */
  fz_try(ctx)
    fz_register_document_handlers(ctx);
  fz_catch(ctx) {
    fprintf(stderr, "WARNING: cannot register document handlers: %s\nskipping %s\n", fz_caught_message(ctx), file_name);
    fz_drop_context(ctx);
    return;
  }
  
  /* Open the document. */
  fz_try(ctx)
    doc = fz_open_document(ctx, file_name);
  fz_catch(ctx) {
    fprintf(stderr, "WARNING: cannot open document: %s\nskipping %s\n", fz_caught_message(ctx), file_name);
    fz_drop_context(ctx);
    return;
  }

  int n_pages;

  fz_try(ctx)
    n_pages = fz_count_pages(ctx, doc);
  fz_catch(ctx) {
    fprintf(stderr, "WARNING: cannot count number of pages: %s\nskipping %s\n", fz_caught_message(ctx), file_name);
    fz_drop_document(ctx, doc);
    fz_drop_context(ctx);
    return;
  }

  mtx = fz_scale(pdf_zoom, pdf_zoom);

  for (int p = 0; p < n_pages; ++p) {

    int n_plots = 0;

    clock_t time_pdf_mupdf_beg = clock();

    fz_try(ctx)
      pix = fz_new_pixmap_from_page_number(ctx, doc, p, mtx, fz_device_gray(ctx), 0);
    fz_catch(ctx) {
      fprintf(stderr, "cannot render page: %s\nskipping page %d in %s\n", fz_caught_message(ctx), p + 1, file_name);
      fz_drop_document(ctx, doc);
      fz_drop_context(ctx);
      continue;
    }

    bm_BitMap *bm = bm_from_pix(pix, threshold);

    bm_BitMap *plots[MAX_PLOTS_PER_PAGE];

    char *page_name = (char *) calloc(strlen(file_name) + 1 + strlen("page_XXXX"), sizeof(char)); //TODO: better name

    sprintf(page_name, "%s_page_%03d", file_name, p + 1);

    #ifdef DEBUG
    bm_print(bm, page_name);
    #endif

    bt_time->pdf_mupdf += (double) (clock() - time_pdf_mupdf_beg);

    clock_t time_pdf_findplots_beg = clock();

    bm_find_plots(bm, plots, &n_plots, MAX_PLOTS_PER_PAGE);

    bt_time->pdf_findplots += (double) (clock() - time_pdf_findplots_beg);

    clock_t time_pdf_loopplots_beg = clock();

    for(int i = 0; i < n_plots; ++i){

      char *plot_name = (char *) calloc(strlen(file_name) + 1 + strlen("page_XXXX_plot_XXXX"), sizeof(char)); //TODO: better name

      sprintf(plot_name, "%s_page_%03d_plot_%03d", file_name, p + 1, i + 1);

      clock_t time_pdf_dct_beg = clock();

      bm_BitMap *dct = bm_discrete_cosine_transform(plots[i], dct_dimension);

      bt_time->pdf_dct += (double) (clock() - time_pdf_dct_beg);

      char *hex = bm_to_hex(dct); //XXX
      printf("%s %s\n", hex, plot_name);

      #ifdef DEBUG
      bm_print(plots[i], plot_name);
      #endif

      db[*n_db].hex = hex;
      db[*n_db].name = plot_name;

      db[*n_db].dist = -1;

      (*n_db)++;
       
      bm_destroy(dct);

    }

    bt_time->pdf_loopplots += (double) (clock() - time_pdf_loopplots_beg);

    bm_destroy(bm);

    for(int i = 0; i < n_plots; ++i)
      bm_destroy(plots[i]);

    fz_drop_pixmap(ctx, pix);

    free(page_name);

  }

  fz_drop_document(ctx, doc);

  fz_drop_context(ctx);

}

void io_add_plots_from_csv(char *file_name, db_Entry db[], int *n_db, int dct_dimension) {

  FILE * in_file = fopen(file_name, "r");

  int hex_length = dct_dimension * dct_dimension / 4;

  while(db_read_entry(in_file, &db[*n_db], hex_length)) {

    printf("%s %s\n", db[*n_db].hex, db[*n_db].name);

    (*n_db)++;

  }

  fclose(in_file);

}
