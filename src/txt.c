#include "txt.h"

void tx_print(char *line, regmatch_t *match, int *n_matches, int *page, char (*this_file)[MAX_MATCHES], char (*this_match)[MAX_MATCHES]) {
  
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

  char *match_beg = line + match->rm_so;
  char *match_end = line + match->rm_eo;

  char *out_beg = match_beg;
  char *out_end = match_end;

  int i_beg = 0;
  int i_end = 0;

  while(out_beg != line && i_beg++ < TXT_OUTPUT_MARGIN)
    out_beg--;

  while(*out_end != '\0' && i_end++ < TXT_OUTPUT_MARGIN)
    out_end++;

  *out_end = '\0';

  char *result = malloc((strlen(out_beg) + strlen(ANSI_YELLOW) + strlen(ANSI_NORM)) * sizeof(char));

  char c;

  c = *match_beg;

  *match_beg = '\0';

  strcpy(result, out_beg);

  *match_beg = c;

  c = *match_end;

  *match_end = '\0';

  strcpy(*this_match, match_beg);

  strcat(result, ANSI_YELLOW);
  strcat(result, match_beg);

  *match_end = c;

  strcat(result, ANSI_NORM);
  strcat(result, match_end);

  strcpy(*this_file, line);

  if(strlen(line) > FILE_OUTPUT_LENGTH)
    line += strlen(line) - FILE_OUTPUT_LENGTH;

  printf("%s%03d%s: %s%s%s %spage %s%s: %s\n", ANSI_PURPLE, *n_matches, ANSI_NORM, ANSI_BLUE, line, ANSI_NORM, ANSI_GREEN, ptr_page, ANSI_NORM, result);

  (*n_matches)++;

  free(result);

  *page = atoi(ptr_page);

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
  
  int pages[MAX_MATCHES];
  char files[MAX_FILE_NAME_LENGTH][MAX_MATCHES];
  char matches[MAX_MATCH_LENGTH][MAX_MATCHES];

  while (fgets(line, MAX_LINE_LENGTH, in_file) != NULL && *n_matches < MAX_MATCHES - 1)
    if(!regexec(&regex, line, 1, &match, 0))
      if(match.rm_eo - match.rm_so < 30) //TODO
        tx_print(line, &match, n_matches, &pages[*n_matches], &(files[*n_matches]), &(matches[*n_matches]));

  fclose(in_file);

  if (*n_matches == 0) {

    printf("no matches found\n");

    return;

  }

  printf("type %smatch number%s to jump to pdf, any other input to quit\n", ANSI_PURPLE, ANSI_NORM);

  int number;

  while(scanf("%d", &number) != 0) {

    printf("You entered: %d\n", number);

    if(number >= 0 && number < * n_matches) {

      char command[1024]; //TODO: dynamically allocate

      sprintf(command, "python3 annotate.py %s %d \"%s\"", files[number], pages[number], matches[number]); //TODO: do this in C

      printf("%s\n", command);

      system(command); //TODO: remove system call

    }
    else
      printf("WARNING: provided number out of range of matches\n");

  }

}
