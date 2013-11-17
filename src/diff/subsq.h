#ifndef SUBSQ_H
#define SUBSQ_H

/* == Inlcudes == */
#include <stdio.h>
#include <stdlib.h>
#include "../constant.h"
#include "diff.h"

/* == Structures/Enum == */
struct _Subsq {
    uint y;

    uint value;

    struct _Subsq *next;
};

typedef struct _Subsq Subsq;

/* == Prototypes == */
void subsq_append(Subsq *s, uint y);
void subsq_free(Subsq s[], uint lenght);
uint subsq_to_index(Subsq *s, Index *i1, Index *i2, uint start);

#endif // SUBSQ_H
