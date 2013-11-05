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
extern short int diff_stderr_show_help;
#else
Params *p = NULL;
short int diff_stderr_show_help = 0;
#endif

#include "err/err.h"

#endif /* CONSTANT_H */
