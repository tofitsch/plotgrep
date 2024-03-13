#ifndef __database_h_
#define __database_h_

typedef struct {

  int doc, page, plot, dist;

} db_Entry;

int db_by_dist(const void*, const void*);

#endif
