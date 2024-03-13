#ifndef __database_h_
#define __database_h_

typedef struct {

  int dist;

  char *name;
  char *hex;

} db_Entry;

int db_by_dist(const void*, const void*t);

void db_destroy_entry(db_Entry*);

#endif
