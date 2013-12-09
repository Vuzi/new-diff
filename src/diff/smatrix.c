#include "smatrix.h"

/* Statiques */
static Smatrix* smatrix_last(Smatrix *s);
static Smatrix* smatrix_at(Smatrix *s, ulint y);
static void smatrix_free_intern(Smatrix *s);

void smatrix_append(Smatrix *s, ulint y) {

    // First cell
    if(s->value == 0) {
        s->y = y;
        s->value = 1;
        s->next = NULL;
    }
    // Other cell to add
    else {
        s = smatrix_last(s);
        s->next = (Smatrix*)malloc(sizeof(Smatrix));
        s->next->y = y;
        s->next->value = 1;
        s->next->next = NULL;
    }

}

static Smatrix* smatrix_last(Smatrix *s) {

    while(s && s->next)
        s = s->next;

    return s;
}


static void smatrix_free_intern(Smatrix *s) {

    Smatrix *tmp = NULL;

    while(s) {
        tmp = s;
        s = s->next;
        free(tmp);
    }
}

void smatrix_free(Smatrix s[], ulint length) {

    for(; length > 0; length--)
        smatrix_free_intern(s[length-1].next);

    free(s);
}

static Smatrix* smatrix_at(Smatrix *s, ulint y) {

    while(s) {
        if(s->value && s->y == y)
            break;
        s = s->next;
    }

    return s;
}

ulint smatrix_to_index(Smatrix s[], Line **LCS_lines[], ulint len[]) {

    ulint i = 0, j = 0, max_length = 0, max_start = 0, changes = 0;
    Smatrix *tmp = NULL;
    ulint tmp_length = 0, tmp_start = 0;

    // Everything to 1
    for(; i < 2; i++) {
        for(j = 0; j < len[i]; j++)
            LCS_lines[i][j]->modified = 1;
    }

    // For every column of the matrix
    for(; i < len[0]; i++) {
        tmp = &s[i];

        // At least one value
        if(tmp->value == 1) {
            max_length = 0; // For the first case

            do {
                tmp_length = 1;
                tmp_start = tmp->y;

                for(j = i+1; j < len[0]; j++) {
                    // We look for the next in diagonal
                    if(smatrix_at(&s[i], tmp_start + tmp_length) != NULL)
                        tmp_length++;
                    else
                        break; // Nothing more, get out
                }

                // If we exceed the max
                if(tmp_length > max_length) {
                    max_length = tmp_length;
                    max_start = tmp_start;
                }


            } while((tmp = tmp->next));

            // Mark those lignes
            for(j = i; j < i + max_length; j++)
                LCS_lines[0][j]->modified = 0;

            for(j = max_start; j < max_start + max_length; j++)
                LCS_lines[1][j]->modified = 0;

            // Go to the next lines
            changes += max_length;
            i += max_length-1;
        }
    }

    return changes;
}
