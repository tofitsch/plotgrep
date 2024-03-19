#include "database.h"

int db_by_min_dist(const void * a, const void * b) {

  const db_EntryPlot * A = (const db_EntryPlot *) a;
  const db_EntryPlot * B = (const db_EntryPlot *) b;

  return A->dist < B->dist;

}

int db_by_max_time(const void * a, const void * b) {

  const db_EntryPage * A = (const db_EntryPage *) a;
  const db_EntryPage * B = (const db_EntryPage *) b;

  return A->time > B->time;

}

void db_write_plot(FILE *f, db_EntryPlot *e){
  
  fputs(e->hex, f);
  fputs(",", f);

  fputs(e->name, f);
  fputs("\n", f);

}

bool db_read_plot(FILE *f, db_EntryPlot *e, int hex_length){
  
  char line[hex_length + MAX_NAME_LENGTH]; //TODO put outside of function/loop
  
  if(fgets(line, sizeof(line), f) == NULL)
    return false;

  line[hex_length] = '\0';

  char *hex = line;
  char *name = line + hex_length + 1;

  name[strlen(name) - 1] = '\0';

  e->hex = (char *) calloc(strlen(hex) + 1, sizeof(char));
  e->name = (char *) calloc(strlen(name) + 1, sizeof(char));

  strcpy(e->hex, hex);
  strcpy(e->name, name);

  return true;

}

void db_destroy_plot(db_EntryPlot *e){

  free(e->hex);
  free(e->name);

}

void db_destroy_page(db_EntryPage *e){

  free(e->file_name);

}
