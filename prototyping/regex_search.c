#include <stdio.h>
#include <regex.h>

#define MAX_LINE_LENGTH 64000
#define RESULT_OUTPUT_LENGTH 128

void print_result(char *line, regmatch_t *match) {
  
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

int main(int argc, char **argv) {

    char *pattern = argv[1];

    regex_t regex;

    int reti = regcomp(&regex, pattern, 0);

    if (reti) {
      fprintf(stderr, "Could not compile regex\n");
      return 1;
    }

    char line[MAX_LINE_LENGTH];

    FILE *in_file  = fopen("atlas.txt", "r");

    regmatch_t match;

    while (fgets(line, MAX_LINE_LENGTH, in_file) != NULL)
      if(!regexec(&regex, line, 1, &match, 0))
        print_result(line, &match);

    regfree(&regex);

    return 0;

}
