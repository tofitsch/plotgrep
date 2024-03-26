#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <mupdf/fitz.h>

#include "io.h"
#include "bitmap.h"
#include "database.h"
#include "txt.h"

#define PDF_ZOOM 2
#define DCT_DIMENSION 16 //TODO: must be divisible by 4 (for hex encoding)
#define MAX_DB_ENTRIES 131072
#define MAX_DB_PAGE_ENTRIES 131072

bt_Time b;
bt_Time *bt_time = &b;

void print_example_usage() {

  fprintf(stderr, "\nexample usage:\n\n");
  fprintf(stderr, " save plots from input pdf to output csv:\n");
  fprintf(stderr, "  ./plotgrep -o output.csv input_file.pdf input_dir/*.pdf\n\n");
  fprintf(stderr, " save text from input pdf to output txt:\n");
  fprintf(stderr, "  ./plotgrep -o output.txt input_file.pdf input_dir/*.pdf\n\n");
  fprintf(stderr, " search screengrab plot in csv:\n");
  fprintf(stderr, "  ./plotgrep input_file.csv input_dir/*.csv\n\n"); 

}

int main(int argc, char **argv) {
  
  bt_init(bt_time);

  clock_t time_main_beg = clock();
  
  int i_arg = 1;
  int arg_offset = 0;

  FILE *out_file_plots = NULL;
  FILE *out_file_text = NULL;

  bm_BitMap *dct_screen_grab = NULL;
  
  db_EntryPlot *db_plots = malloc(MAX_DB_ENTRIES * sizeof(db_EntryPlot));
  int n_db_plots = 0;

  db_EntryPage *db_pages = malloc(MAX_DB_PAGE_ENTRIES * sizeof(db_EntryPage));
  int n_db_pages = 0;

  if (argc < 2) {
    fprintf(stderr, "ERROR: no arguments provided\n");
    print_example_usage();
    exit(EXIT_FAILURE);
  }

  bool input_provided = false;

  char *regex_str = NULL;
  int n_matches = 0;

  for(; i_arg < argc; ++i_arg){

    if(strcmp(argv[i_arg], "-o") == 0) {
      
      if (input_provided) {
        fprintf(stderr, "ERROR: outputs (option '-o') have to be specified before any inputs\n");
        print_example_usage();
        exit(EXIT_FAILURE);
      }

      if (i_arg > argc - 2) {
        fprintf(stderr, "ERROR: output option '-o' given but no output file specified\n");
        print_example_usage();
        exit(EXIT_FAILURE);
      }

      char * file_name = argv[2];
      char * file_extension = file_name + strlen(file_name) - 4;

      if(strcmp(file_extension, ".csv") == 0) {
        printf("output file for plots: %s\n", file_name);
        out_file_plots = fopen(file_name, "w");
      }
      else if(strcmp(file_extension, ".txt") == 0) {
        printf("output file for text: %s\n", file_name);
        out_file_text = fopen(file_name, "w");
      }
      else {
        fprintf(stderr, "ERROR: output file '%s' has invalid extension '%s'. Must be '.csv' or '.txt'\n", file_name, file_extension);
        print_example_usage();
        exit(EXIT_FAILURE);
      }

      i_arg++;
      arg_offset += 2;

    }
    else {

      char *arg = argv[i_arg];
      
      char *file_extension = NULL;

      if(strlen(arg) > 3)
        file_extension = arg + strlen(arg) - 4;

      printf("input %d of %d: %s\n", i_arg - arg_offset, argc - 1 - arg_offset, arg);

      if (file_extension != NULL && strcmp(file_extension, ".pdf") == 0) {

        if (out_file_plots == NULL && out_file_text == NULL) {
          fprintf(stderr, "ERROR: pdf input but no output (option '-o') specified\n");
          print_example_usage();
          exit(EXIT_FAILURE);
        }

        clock_t time_pdf_beg = clock();

        io_read_pdf(arg, out_file_plots, out_file_text, db_plots, &n_db_plots, db_pages, &n_db_pages, DCT_DIMENSION, PDF_ZOOM);

        bt_time->pdf += (double) (clock() - time_pdf_beg);

        input_provided = true;

      }
      else if (file_extension != NULL && strcmp(file_extension, ".csv") == 0) {
      
        io_add_plots_from_csv(arg, out_file_plots, db_plots, &n_db_plots, DCT_DIMENSION);

        input_provided = true;

      }
      else if (file_extension != NULL && strcmp(file_extension, ".txt") == 0) {

        if (regex_str == NULL) {
          fprintf(stderr, "ERROR: Input txt file %s provided but no regex search string\n", arg);
          exit(EXIT_FAILURE);
        }
      
        tx_search(arg, regex_str, &n_matches);

        input_provided = true;

      }
      else {
       
        if (out_file_plots != NULL || out_file_text != NULL) {
          fprintf(stderr, "ERROR: regex search string '%s' provided together with output file (option -o). Regex search requires no output\n", arg);
          exit(EXIT_FAILURE);
        }

        if (input_provided) {
          fprintf(stderr, "ERROR: regex search string '%s' provided after input file. Regex search string must come first\n", arg);
          exit(EXIT_FAILURE);
        }

        if (regex_str != NULL) {
          fprintf(stderr, "ERROR: second regex search string '%s' provided. Only one allowed\n", arg);
          exit(EXIT_FAILURE);
        }

        regex_str = arg;

      }

    }

  }

  if (out_file_plots == NULL && out_file_text == NULL && regex_str == NULL) {
  
    dct_screen_grab = io_get_plot_from_screen_grab(DCT_DIMENSION);

    char *hex = bm_to_hex(dct_screen_grab);

    printf("screen grab: %s\n", hex);

    free(hex);

  }

  if (out_file_plots == NULL && out_file_text == NULL && regex_str == NULL) {

    for(int i = 0; i < n_db_plots ; ++i) {
      
      bm_BitMap *dct_plot = bm_from_hex(db_plots[i].hex, DCT_DIMENSION, DCT_DIMENSION);

      db_plots[i].dist = bm_hamming_distance(dct_plot, dct_screen_grab);

      bm_destroy(dct_plot);

    }
      
    bm_destroy(dct_screen_grab);

    qsort(db_plots, n_db_plots, sizeof(db_EntryPlot), db_by_min_dist);

    #ifdef DEBUG
    for(int i = 0; i < n_db_plots ; ++i)
      printf("%04d %s %s page %d plot %d\n", db_plots[i].dist, db_plots[i].hex, db_plots[i].file_name, db_plots[i].page, db_plots[i].plot);
    #endif

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

  #ifdef DEBUG
  printf("%d pages took longer than %lf s to load\n", n_db_pages - i, min_time_for_print);

  i--;

  for(; i < n_db_pages ; ++i)
    if(db_pages[i].time > min_time_for_print)
      printf("%lf s to load %s page %d\n", db_pages[i].time, db_pages[i].file_name, db_pages[i].page);
  bt_print(bt_time);
  #endif

  for(int i = 0; i < n_db_plots ; ++i)
    db_destroy_plot(&db_plots[i]);

  free(db_plots);

  for(int i = 0; i < n_db_pages ; ++i)
    db_destroy_page(&db_pages[i]);

  free(db_pages);

  if(out_file_plots != NULL)
    fclose(out_file_plots);

  if(out_file_text != NULL)
    fclose(out_file_text);

  bt_time->main += (double) (clock() - time_main_beg);

  return EXIT_SUCCESS;

}
