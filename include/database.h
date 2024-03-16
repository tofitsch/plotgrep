#ifndef __database_h_
#define __database_h_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAX_NAME_LENGTH 100

typedef struct {

  int dist;

  char *name;
  char *hex;

} db_EntryPlot;

typedef struct {

  char *file_name;
  int page;
  double time;

} db_EntryPage;

int db_by_min_dist(const void*, const void*t);
int db_by_max_time(const void*, const void*t);

void db_write_plot(FILE*, db_EntryPlot*);
bool db_read_plot(FILE*, db_EntryPlot *, int);

void db_destroy_plot(db_EntryPlot*);
void db_destroy_page(db_EntryPage*);

#endif
