#include "subsq.h"

/* Statiques */
static Subsq* subsq_last(Subsq *s);
static Subsq* subsq_at(Subsq *s, uint y);
static void subsq_free_intern(Subsq *s);

void subsq_append(Subsq *s, uint y) {

    /* Première case */
    if(s->value == 0) {
        s->y = y;
        s->value = 1;
        s->next = NULL;
    }
    /* Autres case à ajouter */
    else {
        s = subsq_last(s);
        s->next = (Subsq*)malloc(sizeof(Subsq));
        s->next->y = y;
        s->next->value = 1;
        s->next->next = NULL;
    }

}

static Subsq* subsq_last(Subsq *s) {

    while(s && s->next)
        s = s->next;

    return s;
}


static void subsq_free_intern(Subsq *s) {

    Subsq *tmp = NULL;

    while(s) {
        tmp = s;
        s = s->next;
        free(tmp);
    }
}

void subsq_free(Subsq s[], uint lenght) {

    for(; lenght > 0; lenght--)
        subsq_free_intern(s[lenght-1].next);

    free(s);
}

static Subsq* subsq_at(Subsq *s, uint y) {

    while(s) {
        if(s->value && s->y == y)
            break;
        s = s->next;
    }

    return s;
}

uint subsq_to_index(Subsq s[], Index *i1, Index *i2, uint start) {

    uint i = 0, j = 0, max_lenght = 0, max_start = 0;
    Subsq *tmp = NULL, *max_tmp = NULL;
    uint tmp_lenght = 0, tmp_start = 0;

    uint changes = start;

    /* On passe tout à 1 */
    for(; i < i1->line_max; i++)
        i1->lines[i].modified = 1;

    for(; j < i2->line_max; j++)
        i2->lines[j].modified = 1;

    /* Les lignes avant start sont non modifiées */
    for(i = 0; i < start; i++) {
        i1->lines[i].modified = 0;
        i2->lines[i].modified = 0;
    }

    /* Pour chaque cols de la matrice */
    for(; i < i1->line_max; i++) {
        tmp = &s[i];

        /* Si elle contient au moins une valeur */
        if(tmp->value == 1) {

            max_tmp = NULL;
            max_lenght = 0; //Pour être certain que le premier cas est valide

            do {
                tmp_lenght = 1;
                tmp_start = tmp->y;

                for(j = i+1; j < i1->line_max; j++) {
                    /* On cherche le suivant en diagonale */
                    if(subsq_at(&s[i], tmp_start + tmp_lenght) != NULL)
                        tmp_lenght++;
                    else
                        break; // Plus rien, on sort
                }

                /* Si on dépassé le max */
                if(tmp_lenght > max_lenght) {
                    max_lenght = tmp_lenght;
                    max_tmp = tmp;
                    max_start = tmp_start;
                }


            } while((tmp = tmp->next));

            /* On marque les lignes */
            for(j = i; j < i + max_lenght; j++)
                i1->lines[j].modified = 0;

            for(j = max_start; j < max_start + max_lenght; j++)
                i2->lines[j].modified = 0;

            /* On avance */
            changes += max_lenght;
            i += max_lenght-1;
        }
    }

    return changes;
}
