// https://codeberg.org/caps-universe/libcapsraster/src/tag/libcapsraster-1.0.1/test/test-simple-mupdf.c
// https://mupdf.com/docs/examples/example.c
// sudo apt-get install libmupdf-dev ljbig2dec 
// sudo ln -s /usr/lib/x86_64-linux-gnu/libopenjp2.so.2.3.1 /usr/lib/x86_64-linux-gnu/libopenjp2.so
// sudo ln -s /usr/lib/x86_64-linux-gnu/libjbig2dec.so.0 /usr/lib/x86_64-linux-gnu/libjbig2dec.so
// gcc -o x extract_images.c /usr/lib/libmupdf.a /usr/lib/libmupdf-third.a -lmupdf -lm -lmupdf-third -lfreetype -lharfbuzz -ljpeg -lz -L/usr/lib/x86_64-linux-gnu -lopenjp2 -ljbig2dec
// ./x test.pdf
//
#define DEBUG

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <mupdf/fitz.h>

#include "io.h"
#include "bitmap.h"
#include "database.h"

#define PDF_ZOOM 2
#define DCT_DIMENSION 16 //TODO: must be divisible by 4 (for hex encoding)
#define THRESHOLD 200
#define MAX_DB_ENTRIES 65536

bt_Time b;
bt_Time *bt_time = &b;

int main(int argc, char **argv) {
  
  bt_init(bt_time);

  clock_t time_main_beg = clock();
  
  int i_arg = 1;

  FILE * out_file = NULL;

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

    printf("output file: %s\n", file_name);

    out_file = fopen(file_name, "w");

    i_arg += 2;

  }

  if (out_file == NULL) {
  
    dct_screen_grab = io_get_plot_from_screen_grab(DCT_DIMENSION, THRESHOLD);

    char *hex = bm_to_hex(dct_screen_grab);

    printf("screen grab: %s\n", hex);

    free(hex);

  }

  for(; i_arg < argc; ++i_arg){
    
    char * file_name = argv[i_arg];
    char * file_extension = file_name + strlen(file_name) - 4;

    if (out_file == NULL)
      printf("input file %d of %d: %s\n", i_arg, argc - 1, file_name);
    else
      printf("input file %d of %d: %s\n", i_arg - 2, argc - 3, file_name);
    
    if (access(file_name, F_OK) == -1) {
      fprintf(stderr, "input file '%s' does not exist\n", file_name);
      exit(EXIT_FAILURE);
    }

    if(strcmp(file_extension, ".pdf") == 0) {

      clock_t time_pdf_beg = clock();

      io_add_plots_from_pdf(file_name, db, &n_db, DCT_DIMENSION, THRESHOLD, PDF_ZOOM);

      bt_time->pdf += (double) (clock() - time_pdf_beg);

    }
    else if(strcmp(file_extension, ".csv") == 0) {
      io_add_plots_from_csv(file_name, db, &n_db, DCT_DIMENSION);
    }
    else {
      fprintf(stderr, "invalid extension '%s' on input file '%s'. Must be '.csv' or '.pdf'\n", file_extension, file_name);
      exit(EXIT_FAILURE);
    }

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

  bt_time->main += (double) (clock() - time_main_beg);

  bt_print(bt_time);

  return EXIT_SUCCESS;

}
