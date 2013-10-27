#include "diff_list.h"

/* Prototypes fonctions statiques */
static void diff_display_regular(t_diff* list_e, t_index *f1, t_index *f2);
static void diff_display_file_name(t_index *f1);
static void diff_display_context(t_diff* list_e, t_index *f1, t_index *f2);
static void diff_display_unified(t_diff* list_e, t_index *f1, t_index *f2);
static void diff_display_columns(t_diff* list_e, t_index *f1, t_index *f2, int show_max_char);
static void diff_display_columns_common(int start_1, int start_2, unsigned int length, t_index *f1, t_index *f2, unsigned int char_ligne, unsigned int char_center, char op);
static void diff_display_columns_added(int start, unsigned int lenght, t_index *f, unsigned int char_ligne, unsigned int char_center);
static void diff_display_columns_deleted(int start, unsigned int lenght, t_index *f, unsigned int char_ligne, unsigned int char_center);
static void diff_display_ed(t_diff* list_e, t_index *f);
static t_diff* diff_display_ed_get_diff(t_diff* list_e, unsigned int n);

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

        if(diff->type == ADDED_LINE) // Décalage d'ajout de lignes (Les lignes supprimées sont naturellements comptées dans le fichier 1)
                start_1 = ((list_e->start_1 + 1) - p->unifier > 0) ? (list_e->start_1 + 1) - p->unifier : 0 ;
        else
            start_1 = (list_e->start_1 - p->unifier > 0) ? list_e->start_1 - p->unifier : 0 ;

        while( list_e->next && // Tant qu'on a un élement suivant & qu'il est assez prêt du précédent
             ((p->unifier > 0) && ((list_e->next->start_1 - list_e->end_1 - 1 <= (2*p->unifier)) || (list_e->next->start_2 - list_e->end_2 - 1 <= (2*p->unifier))))) {

            list_e = list_e->next;
        }

        /* Fin */
        end_1 = list_e->end_1 + p->unifier >= (signed)(f1->line_max) ? (signed)(f1->line_max-1) : list_e->end_1 + p->unifier;

        if(diff->type == DELETED_LINE && p->unifier == 0) // Cas spécial
            end_2 = list_e->end_2 - 1 + p->unifier >= (signed)(f2->line_max) ? (signed)(f2->line_max-1) : list_e->end_2 - 1 + p->unifier;
        else
            end_2 = list_e->end_2 + p->unifier >= (signed)(f2->line_max) ? (signed)(f2->line_max-1) : list_e->end_2 + p->unifier;

        diff_display_unified_lines(f1, start_1, end_1, f2, start_2, end_2, diff);

        list_e = list_e->next;
    }

}

static void diff_display_columns_common(int start_1, int start_2, unsigned int length, t_index *f1, t_index *f2, unsigned int char_ligne, unsigned int char_center, char op) {

    unsigned int i = 0, j = 0;
    char tmp = 0;

    /* Pour chaque lignes */
    for(i = 0; i < length ; i++) {
        /* On va sur la ligne correspondante */
        line_go_to(f1, i + start_1);
        line_go_to(f2, i + start_2);

        /* Ligne de f1 */
        for(j = 0; j < char_ligne; j++){
            /* On affiche jusqu'à la fin de ligne */
            if((tmp = fgetc(f1->f)) != '\n' && tmp != EOF)
                putchar((int)tmp);
            else
                break;
        }
        /* Ce qui manque en espace */
        for(; j < char_ligne; j++){
            putchar((int)' ');
        }

        /* Opérateur */
        for(j = 0; j < ((char_center-1)/2); j++)
            putchar((int)' ');

        putchar((int)op);

        for(j = 0; j < ((char_center-1)/2) + ((char_center-1)%2); j++)
            putchar((int)' ');

        /* Ligne de f2 */
        for(j = 0; j < char_ligne; j++){
            /* On affiche jusqu'à la fin de ligne */
            if((tmp = fgetc(f2->f)) != '\n' && tmp != EOF)
                putchar((int)tmp);
            else
                break;
        }
        /* Ce qui manque en espace */
        for(; j < char_ligne; j++){
            putchar((int)' ');
        }

        putchar((int)'\n');
    }
}

static void diff_display_columns_added(int start, unsigned int lenght, t_index *f, unsigned int char_ligne, unsigned int char_center) {

    unsigned int i = 0, j = 0;
    char tmp = 0;

    for(i = 0; i < lenght; i++) {
        line_go_to(f, i + start);

        /* Espace vide */
        for(j = 0; j < char_ligne; j++)
            putchar((int)' ');

        /* Opérateur */
        for(j = 0; j < ((char_center-1)/2); j++)
            putchar((int)' ');

        putchar((int)'>');

        for(j = 0; j < ((char_center-1)/2) + ((char_center-1)%2); j++)
            putchar((int)' ');

        /* Ligne ajoutée */
        for(j = 0; j < char_ligne; j++) {
            if((tmp = fgetc(f->f)) != '\n' && tmp != EOF)
                putchar((int)tmp);
            else
                break;
        }
        /* Ce qui manque en espace */
        for(; j < char_ligne; j++){
            putchar((int)' ');
        }

        putchar((int)'\n');
    }
}

static void diff_display_columns_deleted(int start, unsigned int lenght, t_index *f, unsigned int char_ligne, unsigned int char_center) {

    unsigned int i = 0, j = 0;
    char tmp = 0;

    for(i = 0; i < lenght; i++) {
        line_go_to(f, i+start);

        /* Ligne supprimmées */
        for(j = 0; j < char_ligne; j++) {
            if((tmp = fgetc(f->f)) != '\n' && tmp != EOF)
                putchar((int)tmp);
            else
                break;
        }
        /* Ce qui manque en espace */
        for(; j < char_ligne; j++){
            putchar((int)' ');
        }

        /* Opérateur */
        for(j = 0; j < ((char_center-1)/2); j++)
            putchar((int)' ');

        putchar((int)'<');

        for(j = 0; j < ((char_center-1)/2) + ((char_center-1)%2); j++)
            putchar((int)' ');

        /* Espace vide */
        for(j = 0; j < char_ligne; j++)
            putchar((int)' ');

        putchar((int)'\n');
    }
}

static void diff_display_columns(t_diff* list_e, t_index *f1, t_index *f2, int show_max_char) {

    unsigned int char_ligne = (show_max_char - 8) > 0 ? (show_max_char - 8) : 0; // Nombre de colonnes
    unsigned int char_center = show_max_char - (char_ligne); // Nombre de colone centrale

    /* On s'assure d'être revenu au début */
    line_go_to(f1, 0);
    line_go_to(f2, 0);

    if(list_e) {
        /* Tant qu'il reste un élément */
        while(list_e) {

            if(list_e->start_1 - f1->line > 0) // Si des lignes avant le prochain diff, on affiche
                diff_display_columns_common(f1->line, f2->line, list_e->start_1 - f1->line, f1, f2, char_ligne, char_center, ' ');

            if(list_e->type == ADDED_LINE) // Si des lignes ajoutées
                diff_display_columns_added(list_e->start_2, list_e->end_2 - list_e->start_2 + 1, f2, char_ligne, char_center);

            else if(list_e->type == DELETED_LINE) // Si des lignes supprimmées
                diff_display_columns_deleted(list_e->start_1, list_e->end_1 - list_e->start_1 + 1, f1, char_ligne, char_center);

            else { // Si des lignes modifiées

                /* Si plus de ligne dans f1 que dans f2 */
                if(list_e->end_1 - list_e->start_1 > list_e->end_2 - list_e->start_2) {
                    diff_display_columns_common(list_e->start_1,  list_e->start_2, list_e->end_2 - list_e->start_2+1, f1, f2, char_ligne, char_center, '|');
                    diff_display_columns_deleted(list_e->start_1 + (list_e->end_1 - list_e->start_1), (list_e->end_1 - list_e->start_1)-(list_e->end_2 - list_e->start_2), f1, char_ligne, char_center);
                }
                /* Moins ou autant de ligne dans f1 que dans f2 */
                else {
                    diff_display_columns_common(list_e->start_1,  list_e->start_2, list_e->end_1 - list_e->start_1+1, f1, f2, char_ligne, char_center, '|');
                    diff_display_columns_added(f2->line + 1, (list_e->end_2 - list_e->start_2)-(list_e->end_1 - list_e->start_1), f2, char_ligne, char_center);
                }

            }

            line_go_to(f1, list_e->end_1 + 1);
            line_go_to(f2, list_e->end_2 + 1);

            list_e = list_e->next;
        }

        /* Affichage de la fin */
        diff_display_columns_common(f1->line, f2->line, f1->line_max - f1->line - 1, f1, f2, char_ligne, char_center, ' ');

    }
    /* Fichiers identiques, on affiche quand même */
    else
        diff_display_columns_common(0, 0, f1->line_max, f1, f2, char_ligne, char_center, ' ');

}


static t_diff* diff_display_ed_get_diff(t_diff* list_e, unsigned int n) {

    while(n > 0) {
        n--;
        list_e = list_e->next;
    }

    return list_e;
}

static void diff_display_ed(t_diff* list_e, t_index *f) {

    t_diff *tmp = list_e;
    unsigned int diff_len = 0, i = 0;

    /* ed fonctionne à l'envers, on doit garder le nombre de diff disponible */
    while(tmp) {
        tmp = tmp->next;
        diff_len++;
    }

    /* Pour chaque diff */
    for(i = 0; i < diff_len; i++) {
        tmp = diff_display_ed_get_diff(list_e, (diff_len - i) - 1);

        if(tmp->type == ADDED_LINE) {
            if(tmp->end_1 - tmp->start_1 + 1 == 1)
                printf("%da\n", tmp->start_1+1);
            else
                printf("%d,%da\n", tmp->start_1+1, tmp->end_1 - tmp->start_1 + 1);

            lines_display(f,tmp->start_2, tmp->end_2, "");

            puts(".");
        } else if(tmp->type == CHANGED_LINE) {
            if(tmp->end_1 - tmp->start_1 + 1 == 1)
                printf("%dc\n", tmp->start_1+1);
            else
                printf("%d,%dc\n", tmp->start_1+1, tmp->end_1 - tmp->start_1 + 1);

            lines_display(f,tmp->start_2, tmp->end_2, "");

            puts(".");
        } else {
            if(tmp->end_1 - tmp->start_1 + 1 == 1)
                printf("%dd\n", tmp->start_1+1);
            else
                printf("%d,%dd\n", tmp->start_1+1, tmp->end_1 - tmp->start_1 + 1);
        }

    }
}

void diff_display(t_diff* list_e, t_index *f1, t_index *f2) {

    /* Format de sortie contextuel */
    if(p->o_style == CONTEXT)
        diff_display_context(list_e, f1, f2);
    /* Format de sortie unifé */
    else if(p->o_style == UNIFIED)
        diff_display_unified(list_e, f1, f2);
    /* Format edit script */
    else if(p->o_style == EDIT_SCRIPT)
        diff_display_ed(list_e, f2);
    /* Format en colonnes */
    else if(p->o_style == COLUMNS)
        diff_display_columns(list_e, f1, f2, p->show_max_char);
    /* Format pas défaut */
    else
        diff_display_regular(list_e, f1, f2);

}
