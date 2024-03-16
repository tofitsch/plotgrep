#ifndef __io_h_
#define __io_h_

#include "bitmap.h"
#include "database.h"
#include "benchmark.h"

#define DEBUG

#define THRESHOLD 250
#define TMP_FILE "tmp.png"

extern bt_Time *bt_time;

bm_BitMap* io_get_plot_from_screen_grab(int, int);

void io_add_plots_from_pdf(char*, db_Entry [], int*, int, int, int);

void io_add_plots_from_csv(char*, db_Entry [], int*, int);

#endif
