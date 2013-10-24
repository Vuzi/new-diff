#ifndef DIFF_H_INCLUDED
#define DIFF_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define CREATE_PARAMETERS

#include "../params/constant.h"
#include "diff_list.h"

/* Prototypes */
FILE* sec_fopen(const char* path, const char* mode);
void sec_fclose(FILE *f);

int diff_file(const char* f1_name, const char* f2_name);

#endif // DIFF_H_INCLUDED
