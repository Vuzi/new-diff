#include "diff_list.h"

void diff_add(t_diff** list_e, diff_type type, int start_1, int end_1, int start_2, int end_2) {

    t_diff *new_d = (t_diff*)malloc(sizeof(t_diff));

    if(*list_e) {
        diff_last(*list_e)->next = new_d;
    } else {
        *list_e = new_d;
    }

    new_d->next = NULL;
    new_d->start_1 = start_1;
    new_d->start_2 = start_2;
    new_d->end_1 = end_1;
    new_d->end_2 = end_2;
    new_d->type = type;
}

t_diff *diff_last(t_diff* list_e) {

    if(list_e) {
        while(list_e->next) {
            list_e = list_e->next;
        }

        return list_e;
    } else
        return NULL;
}

void diff_delete(t_diff* list) {

    t_diff* tmp = NULL;

    while(list) {
        tmp = list;
        list = list->next;
        free(tmp);
    }
}


void diff_display(t_diff* list_e, t_index *f1, t_index *f2) {

    while(list_e) {

        /* Nous utilisons les lignes de 0 à n-1, là où diff affiche de 1 à n, d'où les +1 */

        /* Ligne et nature modif */
        if(list_e->start_1 != list_e->end_1)
            printf("%d,%d", list_e->start_1+1, list_e->end_1+1);
        else
            printf("%d", list_e->start_1+1);

        if(list_e->type == ADDED_LINE){
            putchar('a');
        }
        else if(list_e->type == CHANGED_LINE){
            putchar('c');
        }
        else{
            putchar('d');
        }

        if(list_e->start_2 != list_e->end_2)
            printf("%d,%d\n", list_e->start_2+1, list_e->end_2+1);
        else
            printf("%d\n", list_e->start_2+1);

        /* Lignes affectées */
        if(list_e->type == ADDED_LINE){
            lines_display(f2, list_e->start_2, list_e->end_2, "> ");
        }
        else if(list_e->type == CHANGED_LINE){
            lines_display(f1, list_e->start_1, list_e->end_1, "< ");
            fputs("---\n", stdout);
            lines_display(f2, list_e->start_2, list_e->end_2, "> ");
        }
        else{
            lines_display(f1, list_e->start_1, list_e->end_1, "< ");
        }

        /* Suite de la liste */
        list_e = list_e->next;
    }

}
