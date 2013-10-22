#ifndef CONSTANT_H
#define CONSTANT_H

/* Standard library */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Personal library */
#include "params.h"

/* Personal global fields */
#ifdef CREATE_PARAMETERS
extern Params *parameter;
#else
Params *parameter = NULL;
#endif

#endif /* CONSTANT_H */