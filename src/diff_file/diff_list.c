#include "diff_list.h"

/* Prototypes fonctions statiques */
static void diff_display_regular(t_diff* list_e, t_index *f1, t_index *f2);
static void diff_display_file_name(t_index *f1);
static void diff_display_context(t_diff* list_e, t_index *f1, t_index *f2);
static void diff_display_unified(t_diff* list_e, t_index *f1, t_index *f2);

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

static void diff_display_regular(t_diff* list_e, t_index *f1, t_index *f2) {

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


static void diff_display_file_name(t_index *f1) {

    int blank_lenght = 0;

    struct stat s;
    char stat_time[512] = {0};
    struct tm *stat_tm = NULL;

    stat(f1->f_name, &s);
    stat_tm = localtime(&(s.st_mtime));
    strftime(stat_time, 512, "%Y-%m-%d %H:%M:%S", stat_tm);

    blank_lenght = 12-(strlen(f1->f_name)%12);

    printf("%s", f1->f_name, s.st_mtime);

    while(blank_lenght > 0) {
        fputc((int)' ', stdout);
        blank_lenght--;
    }

    printf("%s.%llu ", stat_time, (unsigned long long)s.st_mtime);

    strftime(stat_time, 512, "%z", stat_tm);
    printf("%s\n", stat_time);

}

static void diff_display_context_lines(char number, int start, int end, t_diff* diff, t_index *f, const char* esc) {

    /* Fonction sale (code en double, etc...), mais de solution simple pour optmiser... */

    int i = 0;
    char to_display = 1;

    char no_change[3] = "  ";
    char add[3] = "+ ";
    char del[3] = "- ";
    char mod[3] = "! ";

    t_diff *tmp = diff;

    /* On regarde si on doit l'afficher ou non */
    if(diff) {
        if(number == 1) {
            while(tmp->type == ADDED_LINE) {
                if(tmp->next == NULL || tmp->next->start_1 > end) {
                    to_display = 0;
                    break;
                }
                else
                    tmp = tmp->next;
            }

        } else {
            while(tmp->type == DELETED_LINE) {
                if(tmp->next == NULL || tmp->next->start_2 > end) {
                    to_display = 0;
                    break;
                }
                else
                    tmp = tmp->next;
            }

        }
    } else
        to_display = 0;

    /* Affichage des numéros de lignes */
    for(i = 0; i < 3; i++)
        fputs(esc, stdout);

    if(start != end)
        printf(" %d,%d ", start+1, end+1);
    else
        printf(" %d ", start+1);

    for(i = 0; i < 4; i++)
        fputs(esc, stdout);

    fputc('\n', stdout);


    /* Affichage */
    if(to_display) {

        if(number == 1) {

            /* Début du contexte */
            if(start < diff->start_1)
                lines_display(f, start, diff->start_1-1, no_change);

            /* Pour chaque élément */
            while(diff && diff->start_1 <= end) {

                /* Affichage modifications */
                if(diff->type == CHANGED_LINE)
                    lines_display(f, diff->start_1, diff->end_1, mod);
                else if(diff->type == DELETED_LINE)
                    lines_display(f, diff->start_1, diff->end_1, del);

                /* Si espace entre curseur et prochain à afficher */
                if(diff->next && diff->next->start_1 <= end) {
                    lines_display(f, diff->end_1+1, diff->next->start_1-1, no_change);
                } else {
                    lines_display(f, diff->end_1+1, end, no_change);
                }

                diff = diff->next;
            }

        } else {

            /* Début du contexte */
            if(start < diff->start_2)
                lines_display(f, start, diff->start_2-1, no_change);

            /* Pour chaque élément */
            while(diff && diff->start_2 <= end) {

                /* Affichage modifications */
                if(diff->type == CHANGED_LINE)
                    lines_display(f, diff->start_2, diff->end_2, mod);
                else if(diff->type == ADDED_LINE)
                    lines_display(f, diff->start_2, diff->end_2, add);

                /* Si espace entre curseur et prochain à afficher */
                if(diff->next && diff->next->start_2 <= end) {
                    lines_display(f, diff->end_2+1, diff->next->start_2-1, no_change);
                } else {
                    lines_display(f, diff->end_2+1, end, no_change);
                }

                diff = diff->next;
            }
        }
    }

}

static void diff_display_context(t_diff* list_e, t_index *f1, t_index *f2) {

    int start_1;
    int start_2;

    int end_1;
    int end_2;

    t_diff *diff;

    /* Affichage des noms de fichier */
    fputs("*** ",stdout);
    diff_display_file_name(f1);

    fputs("--- ",stdout);
    diff_display_file_name(f2);


    while(list_e) {

        puts("***************");

        diff = list_e;


        /* Début */
        if(diff->type == DELETED_LINE) // Décalage spécial en cas de lignes supp
            start_2 = list_e->start_2;
        else
            start_2 = (list_e->start_2 - p->context > 0) ? list_e->start_2 - p->context : 0 ;

        if(diff->type == ADDED_LINE) // Décalage spécial en cas de lignes ajoutées
            start_1 = list_e->start_1;
        else
            start_1 = (list_e->start_1 - p->context > 0) ? list_e->start_1 - p->context : 0 ;


        while( list_e->next && // Tant qu'on a un élement suivant & qu'il est assez prêt du précédent
             (((p->context > 0) && ((list_e->next->start_1 - list_e->end_1 - 1 < (2*p->context)) || (list_e->next->start_2 - list_e->end_2 - 1 < (2*p->context)))))) {

            list_e = list_e->next;
        }

        /* Fin */
        end_1 = list_e->end_1 + p->context >= (signed)(f1->line_max) ? (signed)(f1->line_max-1) : list_e->end_1 + p->context;
        end_2 = list_e->end_2 + p->context >= (signed)(f2->line_max) ? (signed)(f2->line_max-1) : list_e->end_2 + p->context;

        diff_display_context_lines(1, start_1, end_1, diff, f1, "*");
        diff_display_context_lines(2, start_2, end_2, diff, f2, "-");

        list_e = list_e->next;
    }
}

static void diff_display_unified_lines(t_index *f1, int start_1, int end_1, t_index *f2, int start_2, int end_2, t_diff* diff){

    char no_change[3] = " ";
    char add[3] = "+";
    char del[3] = "-";

    /* Affichage des numéros de lignes */
    if((end_1+1) == 0 && (start_1+1) == 0)
        printf("@@ -0,0");
    else {
        if((end_1+1) - (start_1+1) + 1 == 1)
             printf("@@ -%d", start_1+1);
        else
            printf("@@ -%d,%d", start_1+1, (end_1+1) - (start_1+1) + 1);
    }

    if((end_2+1) == 0 && (start_2+1) == 0)
        printf(" +0,0 @@\n");
    else {
        if((end_2+1) - (start_2+1) + 1 == 1)
             printf(" +%d @@\n", start_2+1);
        else
            printf(" +%d,%d @@\n", start_2+1, (end_2+1) - (start_2+1) + 1);
    }

    if(diff) {

        /* Début du contexte unifié */
        if(start_1 < diff->start_1 || (diff->type == ADDED_LINE && start_1 >= diff->start_1)) {
            if(diff->type == ADDED_LINE)
                lines_display(f1, start_1, diff->start_1, no_change);
            else
                lines_display(f1, start_1, diff->start_1-1, no_change);
        }

            /* Pour chaque élément */
        while(diff && diff->start_1 <= end_1) {

            /* Affichage modifications */
            if(diff->type == CHANGED_LINE) {
                lines_display(f1, diff->start_1, diff->end_1, del);
                lines_display(f2, diff->start_2, diff->end_2, add);
            }
            else if(diff->type == DELETED_LINE)
                lines_display(f1, diff->start_1, diff->end_1, del);
            else if(diff->type == ADDED_LINE)
                lines_display(f2, diff->start_2, diff->end_2, add);


            /* Si espace entre curseur et prochain à afficher */
            if(diff->next && diff->next->start_1 <= end_1) {
                if(diff->next->type == ADDED_LINE)
                    lines_display(f1, diff->end_1+1, diff->next->start_1, no_change);
                else
                    lines_display(f1, diff->end_1+1, diff->next->start_1-1, no_change);
            } else {
                lines_display(f1, diff->end_1+1, end_1, no_change);
            }

            diff = diff->next;
        }
    }

}

static void diff_display_unified(t_diff* list_e, t_index *f1, t_index *f2) {

    int start_1;
    int start_2;

    int end_1;
    int end_2;

    /* Affichage des noms de fichier */
    fputs("--- ",stdout);
    diff_display_file_name(f1);

    fputs("+++ ",stdout);
    diff_display_file_name(f2);

    t_diff *diff;

    while(list_e) {

        diff = list_e;

        /* Début */
        start_2 = (list_e->start_2 - p->unifier > 0) ? list_e->start_2 - p->unifier : 0 ;

        if(diff->type == ADDED_LINE) { // Décalage d'ajout de lignes (Les lignes supprimées sont naturellements comptées dans le fichier 1)
                start_1 = ((list_e->start_1 + 1) - p->unifier > 0) ? (list_e->start_1 + 1) - p->unifier : 0 ;
        }
        else
            start_1 = (list_e->start_1 - p->unifier > 0) ? list_e->start_1 - p->unifier : 0 ;

        while( list_e->next && // Tant qu'on a un élement suivant & qu'il est assez prêt du précédent
             ((p->unifier > 0) && ((list_e->next->start_1 - list_e->end_1 - 1 <= (2*p->unifier)) || (list_e->next->start_2 - list_e->end_2 - 1 <= (2*p->unifier))))) {

            list_e = list_e->next;
        }

        /* Fin */
        end_1 = list_e->end_1 + p->unifier >= (signed)(f1->line_max) ? (signed)(f1->line_max-1) : list_e->end_1 + p->unifier;

        if(diff->type == DELETED_LINE && p->unifier == 0) { // Cas spécial
            end_2 = list_e->end_2 - 1 + p->unifier >= (signed)(f2->line_max) ? (signed)(f2->line_max-1) : list_e->end_2 - 1 + p->unifier;
        } else
            end_2 = list_e->end_2 + p->unifier >= (signed)(f2->line_max) ? (signed)(f2->line_max-1) : list_e->end_2 + p->unifier;

        diff_display_unified_lines(f1, start_1, end_1, f2, start_2, end_2, diff);

        list_e = list_e->next;
    }

}


void diff_display(t_diff* list_e, t_index *f1, t_index *f2) {

    /* Format de sortie contextuel */
    if(p->o_style == CONTEXT) {
        diff_display_context(list_e, f1, f2);
    }
    /* Format de sortie unifé */
    else if(p->o_style == UNIFIED) {
        diff_display_unified(list_e, f1, f2);

    }
    /* Format pas défaut */
    else {
        diff_display_regular(list_e, f1, f2);
    }

}
