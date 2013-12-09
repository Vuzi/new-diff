#ifndef SUBSQ_H
#define SUBSQ_H

/* == Inlcudes == */
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
void smatrix_free(Smatrix s[], ulint length);
ulint smatrix_to_index(Smatrix s[], Line **LCS_lines[], ulint len[]);

#endif // SUBSQ_H
