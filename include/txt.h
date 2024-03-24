#ifndef __txt_h_
#define __txt_h_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>

#define MAX_LINE_LENGTH 64000
#define RESULT_OUTPUT_LENGTH 128

void tx_print(char*, regmatch_t*, int*);
void tx_search(char*, char*, int*);

#endif
