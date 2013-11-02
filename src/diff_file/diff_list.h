#ifndef DIFF_LIST_H_INCLUDED
#define DIFF_LIST_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

#define CREATE_PARAMETERS

#include "../constant.h"
#include "line.h"

/* Liste des différences */

enum diff_type {
    ADDED_LINE = 0, CHANGED_LINE = 1, DELETED_LINE = 2
};

typedef enum diff_type diff_type;

struct s_diff {

    diff_type type;
    int start_1;
    int start_2;
    int end_1;
    int end_2;

    struct s_diff *next;
};

typedef struct s_diff t_diff;


/* Prototypes */
void diff_add(t_diff** list, diff_type type, int start_1, int end_1, int start_2, int end_2);
t_diff *diff_last(t_diff* list);
void diff_display(t_diff* list, t_index *f1, t_index *f2);
void diff_delete(t_diff* list);

#endif // DIFF_LIST_H_INCLUDED
