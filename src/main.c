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
#include <unistd.h>
#include <mupdf/fitz.h>

#include "bitmap.h"
#include "database.h"

#define DEBUG false
#define PDF_ZOOM 2
#define THRESHOLD 200
#define TMP_FILE "tmp.png"
#define DCT_DIMENSION 16 //TODO: must be divisible by 4 (for hex encoding)
#define MAX_DB_ENTRIES 65536

bm_BitMap* get_plot_from_screen_grab() {
  
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

  free(full_command);

  return plots[0];

}

void add_plots_from_pdf(char *file_name, db_Entry db[], int *n_db, int pdf_zoom) {

  fz_context *ctx;
  fz_document *doc;
  fz_pixmap *pix;
  fz_matrix mtx;

  ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
  if (!ctx) {
    fprintf(stderr, "cannot create mupdf context\n");
    exit(EXIT_FAILURE);
  }

  /* Register the default file types to handle. */
  fz_try(ctx)
    fz_register_document_handlers(ctx);
  fz_catch(ctx) {
    fprintf(stderr, "cannot register document handlers: %s\n", fz_caught_message(ctx));
    fz_drop_context(ctx);
    exit(EXIT_FAILURE);
  }
  
  /* Open the document. */
  fz_try(ctx)
    doc = fz_open_document(ctx, file_name);
  fz_catch(ctx) {
    fprintf(stderr, "cannot open document: %s\n", fz_caught_message(ctx));
    fz_drop_context(ctx);
    exit(EXIT_FAILURE);
  }

  printf("reading plots from %s\n", file_name);

  int n_pages;

  fz_try(ctx)
    n_pages = fz_count_pages(ctx, doc);
  fz_catch(ctx) {
    fprintf(stderr, "cannot count number of pages: %s\n", fz_caught_message(ctx));
    fz_drop_document(ctx, doc);
    fz_drop_context(ctx);
    exit(EXIT_FAILURE);
  }

  mtx = fz_scale(pdf_zoom, pdf_zoom);

  for (int p = 0; p < n_pages; ++p) {

    int n_plots = 0;

    fz_try(ctx)
      pix = fz_new_pixmap_from_page_number(ctx, doc, p, mtx, fz_device_gray(ctx), 0);
    fz_catch(ctx) {
      fprintf(stderr, "cannot render page: %s\n", fz_caught_message(ctx));
      fz_drop_document(ctx, doc);
      fz_drop_context(ctx);
      exit(EXIT_FAILURE);
    }

    bm_BitMap *bm = bm_from_pix(pix, THRESHOLD);

    bm_BitMap *plots[MAX_PLOTS_PER_PAGE];

    char *page_name = (char *) calloc(strlen(file_name) + 1 + strlen("page_XXXX"), sizeof(char)); //TODO: better name

    sprintf(page_name, "%s_page_%03d", file_name, p + 1);

    if(DEBUG)
      bm_print(bm, page_name);

    bm_find_plots(bm, plots, &n_plots);

    for(int i = 0; i < n_plots; ++i){

      char *plot_name = (char *) calloc(strlen(file_name) + 1 + strlen("page_XXXX_plot_XXXX"), sizeof(char)); //TODO: better name

      sprintf(plot_name, "%s_page_%03d_plot_%03d", file_name, p + 1, i + 1);

      bm_BitMap *dct = bm_discrete_cosine_transform(plots[i], DCT_DIMENSION);

      char *hex = bm_to_hex(dct); //XXX
      printf("%s %s\n", hex, plot_name);

      if(DEBUG)
        bm_print(plots[i], plot_name);

      db[*n_db].hex = hex;
      db[*n_db].name = plot_name;

      db[*n_db].dist = -1;

      (*n_db)++;
       
      bm_destroy(dct);

    }

    bm_destroy(bm);

    for(int i = 0; i < n_plots; ++i)
      bm_destroy(plots[i]);

    fz_drop_pixmap(ctx, pix);

    free(page_name);

  }

  fz_drop_document(ctx, doc);

  fz_drop_context(ctx);

}

void add_plots_from_csv(char *file_name, db_Entry db[], int *n_db) {

  FILE * in_file = fopen(file_name, "r");

  int hex_length = DCT_DIMENSION * DCT_DIMENSION / 4;

  while(db_read_entry(in_file, &db[*n_db], hex_length)) {

    printf("%s %s\n", db[*n_db].hex, db[*n_db].name);

    (*n_db)++;

  }

  fclose(in_file);

}

int main(int argc, char **argv) {
  
  int i_arg = 1;

  FILE * out_file = NULL;

  bm_BitMap *plot_screen_grab = NULL;
  bm_BitMap *dct_screen_grab = NULL;
  
  db_Entry *db = (db_Entry*) malloc(MAX_DB_ENTRIES * sizeof(db_Entry));
  int n_db = 0;

  if (argc < 2) {
    fprintf(stderr, "example usage:\n\n");
    fprintf(stderr, " save from input PDFs to output CSV:\n");
    fprintf(stderr, "  ./plotgrep -o output.csv input_file.pdf input_dir/*.pdf\n\n");
    fprintf(stderr, " search screengrab in input PDFs and/or CSVs:\n");
    fprintf(stderr, "  ./plotgrep input_file.pdf input_file.csv input_dir/*.pdf input_dir/*.csv\n\n"); 
    exit(EXIT_FAILURE);
  }

  if(strcmp(argv[1], "-o") == 0) {

    if(argc < 3) {
      fprintf(stderr, "output option '-o' given but no output file specified\n");
      exit(EXIT_FAILURE);
    }

    char * file_name = argv[2];
    char * file_extension = file_name + strlen(file_name) - 4;

    if(strcmp(file_extension, ".csv") != 0) {
      fprintf(stderr, "output file '%s' has invalid extension '%s'. Must be '.csv'\n", file_name, file_extension);
      exit(EXIT_FAILURE);
    }

    printf("will write to %s\n", file_name);

    out_file = fopen(file_name, "w");

    i_arg += 2;

  }

  if (out_file == NULL) {
  
    plot_screen_grab = get_plot_from_screen_grab();

    dct_screen_grab = bm_discrete_cosine_transform(plot_screen_grab, DCT_DIMENSION);

    char *hex = bm_to_hex(dct_screen_grab);

    printf("screen grab: %s\n", hex);

    free(hex);

  }

  for(; i_arg < argc; ++i_arg){
    
    char * file_name = argv[i_arg];
    char * file_extension = file_name + strlen(file_name) - 4;

    if (access(file_name, F_OK) == -1) {
      fprintf(stderr, "input file '%s' does not exist\n", file_name);
      exit(EXIT_FAILURE);
    }

    if(strcmp(file_extension, ".pdf") == 0)
      add_plots_from_pdf(file_name, db, &n_db, PDF_ZOOM);

    if(strcmp(file_extension, ".csv") == 0)
      add_plots_from_csv(file_name, db, &n_db);

  }

  if(out_file != NULL)
    for(int i = 0; i < n_db ; ++i)
      db_write_entry(out_file, &db[i]);

  if (out_file == NULL) {

    for(int i = 0; i < n_db ; ++i) {
      
      bm_BitMap *dct_plot = bm_from_hex(db[i].hex, DCT_DIMENSION, DCT_DIMENSION);

      db[i].dist = bm_hamming_distance(dct_plot, dct_screen_grab);

      bm_destroy(dct_plot);

    }
      
    bm_destroy(plot_screen_grab);
    bm_destroy(dct_screen_grab);

    qsort(db, n_db, sizeof(db_Entry), db_by_dist);

    for(int i = 0; i < n_db ; ++i)
      printf("%04d %s %s\n", db[i].dist, db[i].hex, db[i].name);

  }

  for(int i = 0; i < n_db ; ++i)
    db_destroy_entry(&db[i]);

  free(db);

  if(out_file != NULL)
    fclose(out_file);

  return EXIT_SUCCESS;

}
