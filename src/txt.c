#include "txt.h"

void tx_print(char *line, regmatch_t *match) {
  
  char *ptr = line;

  while(*ptr != ',')
    ptr++;

  *ptr = '\0';

  ptr += 7;

  char *ptr_page = ptr;

  while(*ptr != ':')
    ptr++;

  *ptr = '\0';

  char *result = line + match->rm_so;

  ptr = line + match->rm_eo;

  for (int i = 0; i < RESULT_OUTPUT_LENGTH / 2 - (match->rm_eo - match->rm_so); ++i) {

    if(result != line)
      result--;

    if(*ptr != '\0' && *ptr != '\n')
      ptr++;

  }

  *ptr = '\0';

  printf("%s page %s: '%s'\n", line, ptr_page, result);

}

void tx_search(char *in_file_name, char *pattern) {

  if (access(in_file_name, F_OK) == -1) {
    fprintf(stderr, "WARNING: input file '%s' does not exist\n", in_file_name);
    return;
  }

  regex_t regex;

  int reti = regcomp(&regex, pattern, 0);

  if (reti) {
    fprintf(stderr, "could not compile regex\n");
    exit(EXIT_FAILURE);
  }

  char line[MAX_LINE_LENGTH];

  FILE *in_file  = fopen(in_file_name, "r");

  regmatch_t match;

  while (fgets(line, MAX_LINE_LENGTH, in_file) != NULL)
    if(!regexec(&regex, line, 1, &match, 0))
      tx_print(line, &match);

  regfree(&regex);

  fclose(in_file);

}
