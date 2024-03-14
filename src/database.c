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

void db_write_entry(FILE *f, db_Entry *e){
  
  fputs(e->hex, f);
  fputs(",", f);

  fputs(e->name, f);
  fputs("\n", f);

}

bool db_read_entry(FILE *f, db_Entry *e, int hex_length, char *buff){

  if(fgets(buff, sizeof(buff), f) == NULL)
    return false;

  buff[hex_length] = '\0';

  strcpy(e->hex, buff);
  strcpy(e->name, buff + hex_length + 1);

  return true;

}
