#include "database.h"

int db_by_dist(const void * a, const void * b) {

  const db_Entry * A = (const db_Entry *) a;
  const db_Entry * B = (const db_Entry *) b;

  return A->dist < B->dist;

}

void db_write_entry(FILE *f, db_Entry *e){
  
  fputs(e->hex, f);
  fputs(",", f);

  fputs(e->name, f);
  fputs("\n", f);

}

bool db_read_entry(FILE *f, db_Entry *e, int hex_length){
  
  char line[hex_length + MAX_NAME_LENGTH]; //TODO put outside of function/loop
  
  if(fgets(line, sizeof(line), f) == NULL)
    return false;

  line[hex_length] = '\0';

  char *hex = line;
  char *name = line + hex_length + 1;

  name[strlen(name) - 1] = '\0';

  e->hex = (char *) calloc((strlen(hex)), sizeof(char));
  e->name = (char *) calloc((strlen(name)), sizeof(char));

  strcpy(e->hex, hex);
  strcpy(e->name, name);

  printf("%s %s\n", e->hex, e->name);

  return true;

}

void db_destroy_entry(db_Entry *e){

  free(e->hex);
  free(e->name);

}
