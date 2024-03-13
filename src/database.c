#include "database.h"

int db_by_dist(const void * a, const void * b) {

  const db_Entry * A = (const db_Entry *) a;
  const db_Entry * B = (const db_Entry *) b;

  return A->dist < B->dist;

}

void db_destroy_entry(db_Entry *e){

  free(e->hex);
  free(e->name);

}
