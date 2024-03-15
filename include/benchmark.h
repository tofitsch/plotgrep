#ifndef __benchmark_h_
#define __benchmark_h_

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

typedef struct {

  double main, pdf, pdf_loopplots, pdf_findplots, pdf_mupdf, pdf_dct;

} bt_Time;

void bt_init(bt_Time*);
void bt_print(bt_Time*);

#endif
