#ifndef __io_h_
#define __io_h_

#include <unistd.h>

#include "bitmap.h"
#include "database.h"
#include "benchmark.h"

//#define DEBUG

#define TMP_FILE "tmp.png"

extern bt_Time *bt_time;

bm_BitMap* io_get_plot_from_screen_grab(int);

void io_read_pdf(char*, FILE*, FILE*, db_EntryPlot [], int*, db_EntryPage [], int*, int, int);

void io_add_plots_from_csv(char*, FILE*, db_EntryPlot [], int*, int);

#endif
