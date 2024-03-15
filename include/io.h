#ifndef __io_h_
#define __io_h_

#include "bitmap.h"
#include "database.h"

#define THRESHOLD 200
#define TMP_FILE "tmp.png"

bm_BitMap* io_get_plot_from_screen_grab(int);

void io_add_plots_from_pdf(char*, db_Entry [], int*, int, int);

void io_add_plots_from_csv(char*, db_Entry [], int*, int);

#endif
