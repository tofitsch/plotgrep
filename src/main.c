// https://codeberg.org/caps-universe/libcapsraster/src/tag/libcapsraster-1.0.1/test/test-simple-mupdf.c
// https://mupdf.com/docs/examples/example.c
// sudo apt-get install libmupdf-dev ljbig2dec 
// sudo ln -s /usr/lib/x86_64-linux-gnu/libopenjp2.so.2.3.1 /usr/lib/x86_64-linux-gnu/libopenjp2.so
// sudo ln -s /usr/lib/x86_64-linux-gnu/libjbig2dec.so.0 /usr/lib/x86_64-linux-gnu/libjbig2dec.so
// gcc -o x extract_images.c /usr/lib/libmupdf.a /usr/lib/libmupdf-third.a -lmupdf -lm -lmupdf-third -lfreetype -lharfbuzz -ljpeg -lz -L/usr/lib/x86_64-linux-gnu -lopenjp2 -ljbig2dec
// ./x test.pdf
//
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
#define MAX_DB_ENTRIES 131072
#define MAX_DB_PAGE_ENTRIES 131072

bt_Time b;
bt_Time *bt_time = &b;

int main(int argc, char **argv) {
  
  bt_init(bt_time);

  clock_t time_main_beg = clock();
  
  int i_arg = 1;

  FILE *out_file = NULL;

  bm_BitMap *dct_screen_grab = NULL;
  
  db_EntryPlot *db_plots = (db_EntryPlot*) malloc(MAX_DB_ENTRIES * sizeof(db_EntryPlot));
  int n_db_plots = 0;

  db_EntryPage *db_pages = (db_EntryPage*) malloc(MAX_DB_PAGE_ENTRIES * sizeof(db_EntryPage));
  int n_db_pages = 0;

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
  
    dct_screen_grab = io_get_plot_from_screen_grab(DCT_DIMENSION);

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
      fprintf(stderr, "WARNING: input file '%s' does not exist\n", file_name);
      continue;
    }

    if(strcmp(file_extension, ".pdf") == 0) {

      clock_t time_pdf_beg = clock();

      io_add_plots_from_pdf(file_name, out_file, db_plots, &n_db_plots, db_pages, &n_db_pages, DCT_DIMENSION, PDF_ZOOM);

      bt_time->pdf += (double) (clock() - time_pdf_beg);

    }
    else if(strcmp(file_extension, ".csv") == 0) {
      io_add_plots_from_csv(file_name, out_file, db_plots, &n_db_plots, DCT_DIMENSION);
    }
    else {
      fprintf(stderr, "WARNING: invalid extension '%s' on input file '%s'. Must be '.csv' or '.pdf'\n", file_extension, file_name);
      continue;
    }

  }

  if (out_file == NULL) {

    for(int i = 0; i < n_db_plots ; ++i) {
      
      bm_BitMap *dct_plot = bm_from_hex(db_plots[i].hex, DCT_DIMENSION, DCT_DIMENSION);

      db_plots[i].dist = bm_hamming_distance(dct_plot, dct_screen_grab);

      bm_destroy(dct_plot);

    }
      
    bm_destroy(dct_screen_grab);

    qsort(db_plots, n_db_plots, sizeof(db_EntryPlot), db_by_min_dist);

    for(int i = 0; i < n_db_plots ; ++i)
      printf("%04d %s %s page %d plot %d\n", db_plots[i].dist, db_plots[i].hex, db_plots[i].file_name, db_plots[i].page, db_plots[i].plot);

    char command[128]; //TODO: dynamically allocate

    int page = db_plots[n_db_plots - 1].page;
    char *file_name = db_plots[n_db_plots - 1].file_name;
    
    sprintf(command, "zathura --page %d %s", page, file_name);

    system(command); //TODO: remove system call

  }

  qsort(db_pages, n_db_pages, sizeof(db_EntryPage), db_by_max_time);

  double min_time_for_print = 1.;

  int i = 0;

  for(; i < n_db_pages ; ++i)
    if(db_pages[i].time > min_time_for_print)
      break;

  printf("%d pages took longer than %lf s to load\n", n_db_pages - i, min_time_for_print);

  i--;

  for(; i < n_db_pages ; ++i)
    if(db_pages[i].time > min_time_for_print)
      printf("%lf s to load %s page %d\n", db_pages[i].time, db_pages[i].file_name, db_pages[i].page);

  for(int i = 0; i < n_db_plots ; ++i)
    db_destroy_plot(&db_plots[i]);

  free(db_plots);

  for(int i = 0; i < n_db_pages ; ++i)
    db_destroy_page(&db_pages[i]);

  free(db_pages);

  if(out_file != NULL)
    fclose(out_file);

  bt_time->main += (double) (clock() - time_main_beg);

  bt_print(bt_time);

  return EXIT_SUCCESS;

}
