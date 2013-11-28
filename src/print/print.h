#ifndef _H_PRINT
#define _H_PRINT

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../constant.h"
#include "../diff/diff.h"

void print_diff(File files[]);
void print_args(void);
void print_version(void);
void print_help(void);

#endif // _H_PRINT
