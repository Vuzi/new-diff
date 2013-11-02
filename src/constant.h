#ifndef CONSTANT_H
#define CONSTANT_H

/* libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "params/params.h"

/* Personal global fields */
#ifdef CREATE_PARAMETERS
extern Params *p;
extern short int diff_stderr;
extern short int diff_stderr_show_help;
extern char diff_stderr_title[512];
extern char diff_stderr_msg[1024];
#else
Params *p = NULL;
short int diff_stderr = 0;
short int diff_stderr_show_help = 0;
char diff_stderr_title[512] = {0};
char diff_stderr_msg[1024] = {0};
#endif

#include "err.h"

#endif /* CONSTANT_H */
