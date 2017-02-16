#ifndef PARSE_LINE_H
#define PARSE_LINE_H

#include <ctype.h>
#include <stdio.h>
#include <string.h>


int line_byte_cnt(FILE *);
char * clean_line(char *);
void rm_end_space(char *);
char * rm_space(char *);
int check_list(char *);
int cnt_elements(const char *, const char *d);
int comp_string(const void *, const void *);
void parse_string(const char *, char **, const char *);
char *sanitize_num(char *);

#endif
