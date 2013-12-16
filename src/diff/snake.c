#include "snake.h"


/* ===============================================
                    snake_add

    Add a snake on top of the stack pointed
    by snakes. Snakes is modified to point to
    the last snake added.
    ----------------------------------------------
    Snake **snakes : stack of snakes
    l int x_start  : X of the start position
    l int y_start  : Y of the start position
    l int x_mid    : X of the middle position
    l int y_mid    : Y of the middle position
    l int x_end    : X of the end position
    l int y_end    : Y of the end position
   =============================================== */
void snake_add( Snake **snakes, lint x_start, lint y_start, lint x_mid, lint y_mid, lint x_end, lint y_end) {

    Snake *new_snake = diff_xmalloc(sizeof(Snake));

    // Values
    new_snake->start.x = x_start;
    new_snake->start.y = y_start;
    new_snake->mid.x = x_mid;
    new_snake->mid.y = y_mid;
    new_snake->end.x = x_end;
    new_snake->end.y = y_end;

    // Links
    new_snake->next = *snakes;
    new_snake->path = NULL;
    *snakes = new_snake;
}


/* ===============================================
                    snake_clear

    Clear the stack of snakes pointed by snakes.
    ----------------------------------------------
    Snake *snakes : stack of snakes
   =============================================== */
void snake_clear(Snake *snakes) {

    Snake *to_del = NULL;

    while(snakes) {
        to_del = snakes;
        snakes = snakes->next;
        free(to_del);
    }
}
