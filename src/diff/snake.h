#ifndef SNAKE_H
#define SNAKE_H

/* == Inlcudes == */
#include "../constant.h"
#include "diff.h"

/* == Structures/Enum == */
struct _Point {
    lint x;
    lint y;
};

typedef struct _Point Point;

struct _Snake {

    Point start;
    Point mid;
    Point end;

    struct _Snake *next; // Chained list
    struct _Snake *path; // Path to 0,0
};

typedef struct _Snake Snake;

/* == Prototypes == */
void snake_add( Snake **snakes, lint x_start, lint y_start, lint x_mid, lint y_mid, lint x_end, lint y_end);
void snake_clear(Snake *snakes);

#endif // SNAKE_H
