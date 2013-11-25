#ifndef SUBSQ_H
#define SUBSQ_H

/* == Inlcudes == */
#include <stdio.h>
#include <stdlib.h>
#include "../constant.h"
#include "diff.h"

/* == Structures/Enum == */
struct _Smatrix {
    ulint y;

    ulint value;

    struct _Smatrix *next;
};

typedef struct _Smatrix Smatrix;

/* == Prototypes == */
void smatrix_append(Smatrix *s, ulint y);
void smatrix_free(Smatrix s[], ulint lenght);
ulint smatrix_to_index(Smatrix *s, Index *i1, Index *i2, ulint start);

#endif // SUBSQ_H
