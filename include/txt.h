#ifndef __txt_h_
#define __txt_h_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>

#define MAX_LINE_LENGTH 64000
#define MAX_FILE_NAME_LENGTH 1024
#define MAX_MATCH_LENGTH 1024
#define MAX_MATCHES 1024
#define FILE_OUTPUT_LENGTH 32
#define TXT_OUTPUT_MARGIN 32
#define ANSI_RED "\033[1;31m"
#define ANSI_GREEN "\033[1;32m"
#define ANSI_YELLOW "\033[1;33m"
#define ANSI_BLUE "\033[1;34m"
#define ANSI_PURPLE "\033[1;35m"
#define ANSI_NORM "\033[0m"

void tx_print(char*, regmatch_t*, int*, int*, char (*)[MAX_MATCHES], char (*)[MAX_MATCHES]);
void tx_search(char*, char*, int*);

#endif
