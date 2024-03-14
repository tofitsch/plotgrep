#ifndef __database_h_
#define __database_h_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

typedef struct {

  int dist;

  char *name;
  char *hex;

} db_Entry;

int db_by_dist(const void*, const void*t);

void db_write_entry(FILE*, db_Entry*);
bool db_read_entry(FILE*, db_Entry *, int, char*);

void db_destroy_entry(db_Entry*);

#endif
