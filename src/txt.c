#include "txt.h"

void tx_print(char *line, regmatch_t *match, int *n_matches) {
  
  char *ptr = line;

  while(*ptr != ',')
    ptr++;

  *ptr = '\0';

  ptr += 7;

  char *ptr_page = ptr;

  while(*ptr != ':')
    ptr++;

  *ptr = '\0';

  ptr++;

  size_t match_centre = match->rm_so + (match->rm_eo - match->rm_so) / 2;

  char *result_beg = line + match_centre;

  char *result_end = NULL;

  if((match_centre + RESULT_OUTPUT_LENGTH / 2) < strlen(ptr))
    result_end = result_beg + RESULT_OUTPUT_LENGTH / 2;
  else
    result_end = ptr + strlen(ptr) - 1;

  *result_end = '\0';

  if(result_beg - RESULT_OUTPUT_LENGTH / 2 > ptr)
    result_beg -= RESULT_OUTPUT_LENGTH / 2;
  else
    result_beg = ptr;

  printf("%03d: %s page %s: '%s'\n", *n_matches, line, ptr_page, result_beg);

  (*n_matches)++;

}

void tx_search(char *in_file_name, char *pattern, int *n_matches) {

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
      tx_print(line, &match, n_matches);

  regfree(&regex);

  fclose(in_file);

}
