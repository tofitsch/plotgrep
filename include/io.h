#ifndef __io_h_
#define __io_h_

#include "bitmap.h"
#include "database.h"

#define THRESHOLD 200
#define TMP_FILE "tmp.png"

extern double time_pdf_loopplots_beg, time_pdf_findplots_beg, time_pdf_mupdf_beg, time_pdf_dct_beg;
extern double time_pdf_loopplots_end, time_pdf_findplots_end, time_pdf_mupdf_end, time_pdf_dct_end;
extern double time_pdf_loopplots_sum, time_pdf_findplots_sum, time_pdf_mupdf_sum, time_pdf_dct_sum;

bm_BitMap* io_get_plot_from_screen_grab(int, int);

void io_add_plots_from_pdf(char*, db_Entry [], int*, int, int, int);

void io_add_plots_from_csv(char*, db_Entry [], int*, int);

#endif
