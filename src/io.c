#include "io.h"

bm_BitMap* io_get_plot_from_screen_grab(int dct_dimension) {

  printf("select plot to search\n");

  char command[] = "import -depth 8 -colorspace gray ";

  char *full_command = (char *) calloc((strlen(command) + strlen(TMP_FILE) + 1), sizeof(char));

  strcat(full_command, command);
  strcat(full_command, TMP_FILE);

  system(full_command); //TODO: remove system call

  free(full_command);

  FILE * in_file = fopen(TMP_FILE, "r");

  unsigned char header[PNG_HEADER_BYTES];

  fread(header, 1, PNG_HEADER_BYTES, in_file);

  if (png_sig_cmp(header, 0, PNG_HEADER_BYTES)) {
    fprintf(stderr, "Error: not a valid PNG file\n");
    return NULL;
  }

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr) {
    fprintf(stderr, "Error: png_create_read_struct failed\n");
    return NULL;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);

  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    fprintf(stderr, "Error: png_create_info_struct failed\n");
    return NULL;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fprintf(stderr, "Error: Error during PNG read\n");
    return NULL;
  }

  png_init_io(png_ptr, in_file);
  png_set_sig_bytes(png_ptr, PNG_HEADER_BYTES);
  png_read_info(png_ptr, info_ptr);

  png_byte color_type = png_get_color_type(png_ptr, info_ptr);
  png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  if (color_type != PNG_COLOR_TYPE_GRAY || bit_depth != 8) {
    fprintf(stderr, "Error: Only 8-bit grayscale PNG images are supported\nUse `import -depth 8 -colorspace gray test.png`");
    return NULL;
  }

  int w = png_get_image_width(png_ptr, info_ptr);
  int h = png_get_image_height(png_ptr, info_ptr);

  png_bytep *png_bytes = (png_bytep *) malloc(h * sizeof(png_bytep));

  for (int y = 0; y < h; ++y) {

    png_bytes[y] = (png_bytep) malloc(png_get_rowbytes(png_ptr, info_ptr));

    png_read_row(png_ptr, png_bytes[y], NULL);

  }

  bm_BitMap *bm = bm_from_png(png_bytes, w, h);

  #ifdef DEBUG
  bm_print(bm, "pngtest");
  #endif
  
  int n_plots = 0;
  bm_BitMap *plots[MAX_PLOTS_PER_PAGE];

  bm_find_plots(bm, plots, &n_plots, MAX_PLOTS_PER_PAGE, NULL, png_bytes); //TODO

  qsort(plots, n_plots, sizeof(bm_BitMap), bm_by_area);

  for (int y = 0; y < h; ++y)
    free(png_bytes[y]);

  free(png_bytes);

  remove(TMP_FILE); //TODO: remove system call

  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

  fclose(in_file);

  bm_destroy(bm);

  if (n_plots == 0) {
    fprintf(stderr, "cannot find plot in screen grab\n");
    exit(EXIT_FAILURE);
  }

  #ifdef DEBUG
  bm_print(plots[0], "pngtest_plot");
  #endif

  bm_BitMap *dct_screen_grab = bm_discrete_cosine_transform(plots[0], dct_dimension);

  #ifdef DEBUG
  bm_print(dct_screen_grab, "pngtest_dct");
  #endif

  bm_destroy(plots[0]);

  return dct_screen_grab;

}

void io_add_plots_from_pdf(char *file_name, FILE *out_file, db_EntryPlot db_plots[], int *n_db_plots, db_EntryPage db_pages[], int *n_db_pages, int dct_dimension, int pdf_zoom) {

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

    clock_t time_pdf_mupdf_beg = clock();

    db_pages[*n_db_pages].file_name = (char *) calloc(strlen(file_name), sizeof(char));

    strcpy(db_pages[*n_db_pages].file_name, file_name);
    db_pages[*n_db_pages].page = p + 1;
    db_pages[*n_db_pages].time = -1.;

    (*n_db_pages)++;

    int n_plots = 0;

    fz_try(ctx)
      pix = fz_new_pixmap_from_page_number(ctx, doc, p, mtx, fz_device_gray(ctx), 0);
    fz_catch(ctx) {
      fprintf(stderr, "WARNING: cannot render page: %s\nskipping page %d in %s\nskipping %s\n", fz_caught_message(ctx), p + 1, file_name, file_name);
      fz_drop_document(ctx, doc);
      fz_drop_context(ctx);
      return;
    }

    bm_BitMap *bm = bm_from_pdf(pix);

    bm_BitMap *plots[MAX_PLOTS_PER_PAGE];

    char *page_name = (char *) calloc(strlen(file_name) + 1 + strlen("page_XXXX"), sizeof(char)); //TODO: better name

    sprintf(page_name, "%s_page_%03d", file_name, p + 1);

    #ifdef DEBUG
    bm_print(bm, page_name);
    #endif

    bt_time->pdf_mupdf += (double) (clock() - time_pdf_mupdf_beg);

    clock_t time_pdf_findplots_beg = clock();

    bm_find_plots(bm, plots, &n_plots, MAX_PLOTS_PER_PAGE, pix, NULL);

    qsort(plots, n_plots, sizeof(bm_BitMap), bm_by_area);

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

      db_plots[*n_db_plots].hex = hex;
      db_plots[*n_db_plots].file_name = plot_name;

      db_plots[*n_db_plots].dist = -1;

      if(out_file != NULL)
        db_write_plot(out_file, &db_plots[*n_db_plots]);

      (*n_db_plots)++;
       
      bm_destroy(dct);

    }

    bt_time->pdf_loopplots += (double) (clock() - time_pdf_loopplots_beg);

    bm_destroy(bm);

    for(int i = 0; i < n_plots; ++i)
      bm_destroy(plots[i]);

    fz_drop_pixmap(ctx, pix);

    free(page_name);

    db_pages[*n_db_pages - 1].time = (double) (clock() - time_pdf_mupdf_beg) / (double) CLOCKS_PER_SEC;

  }

  fz_drop_document(ctx, doc);

  fz_drop_context(ctx);

}

void io_add_plots_from_csv(char *file_name, FILE *out_file, db_EntryPlot db_plots[], int *n_db_plots, int dct_dimension) {

  FILE * in_file = fopen(file_name, "r");

  int hex_length = dct_dimension * dct_dimension / 4;

  while(db_read_plot(in_file, &db_plots[*n_db_plots], hex_length)) {

    printf("%03d %s %s\n", *n_db_plots, db_plots[*n_db_plots].hex, db_plots[*n_db_plots].file_name);

    if(out_file != NULL)
      db_write_plot(out_file, &db_plots[*n_db_plots]);

    (*n_db_plots)++;

  }

  fclose(in_file);

}
