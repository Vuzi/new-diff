#ifndef LINE_H_INCLUDED
#define LINE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

#include "index.h"

/* Possibles erreurs */
enum line_error {
    _NO_ERROR = 0, END_FILE = 1, START_FILE = 2, CANT_READ_FILE = 3, EMPTY_STRUCT = 4
};

typedef enum line_error line_error;

/* Prototypes */
line_error line_next(t_index *f);
line_error line_previous(t_index *f);
line_error line_go_to(t_index *f, unsigned int n);

int line_compare(t_index *l1, t_index *l2);
int line_search(t_index *line, t_index *f1);
void lines_display(t_index *f, unsigned int start, unsigned int end, const char *line_start);

#endif // LINE_H_INCLUDED
