#include "benchmark.h"

void bt_init(bt_Time *bt) {

  bt->main = 0.;
  bt->pdf = 0.;
  bt->pdf_loopplots = 0.;
  bt->pdf_findplots = 0.;
  bt->pdf_mupdf = 0.;
  bt->pdf_dct = 0.;

}

void bt_print(bt_Time *bt) {

  printf("time_main: %f s\n", bt->main / ((double) CLOCKS_PER_SEC));
  printf("time_pdf: %f %%\n", bt->pdf / bt->main * 100.);
  printf("time_pdf_mupdf: %f %%\n", bt->pdf_mupdf / bt->main * 100.);
  printf("time_pdf_findplots: %f %%\n", bt->pdf_findplots / bt->main * 100.);
  printf("time_pdf_loopplots: %f %%\n", bt->pdf_loopplots / bt->main * 100.);
  printf("time_pdf_dct: %f %%\n", bt->pdf_dct / bt->main * 100.);

}

