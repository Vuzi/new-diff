#include "diff_list.h"

/* Prototypes fonctions statiques */
static void diff_display_regular(t_diff* list_e, t_index *f1, t_index *f2);
static void diff_display_file_name(t_index *f);
static void diff_display_end_of_file(unsigned int end, t_index *f, short int show_msg);
static void diff_display_current_c_func(t_index* index, unsigned int line);
static void diff_display_context(t_diff* list_e, t_index *f1, t_index *f2);
static void diff_display_unified(t_diff* list_e, t_index *f1, t_index *f2);
static void diff_display_columns(t_diff* list_e, t_index *f1, t_index *f2, int show_max_char);
static void diff_display_columns_common(int start_1, int start_2, unsigned int length, t_index *f1, t_index *f2, unsigned int char_ligne, unsigned int char_center, char op);
static void diff_display_columns_added(int start, unsigned int lenght, t_index *f, unsigned int char_ligne, unsigned int char_center);
static void diff_display_columns_deleted(int start, unsigned int lenght, t_index *f, unsigned int char_ligne, unsigned int char_center);
static void diff_display_ed(t_diff* list_e, t_index *f);
static t_diff* diff_display_ed_get_diff(t_diff* list_e, unsigned int n);


/* ===============================================
                    diff_add

    Ajoute un élément à la liste list. Si la liste
    est vide, elle sera crée.
    ----------------------------------------------
    t_diff** list  : liste à traiter
    diff_type type : type de différence
    int start_1    : début changement f1
    int end_1      : fin changement f1
    int start_2    : début changement f2
    int end_2      : fin changement f2
    ----------------------------------------------
   =============================================== */
void diff_add(t_diff** list, diff_type type, int start_1, int end_1, int start_2, int end_2) {

    t_diff *new_d = (t_diff*)malloc(sizeof(t_diff));

    if(*list) {
        diff_last(*list)->next = new_d;
    } else {
        *list = new_d;
    }

    new_d->next = NULL;
    new_d->start_1 = start_1;
    new_d->start_2 = start_2;
    new_d->end_1 = end_1;
    new_d->end_2 = end_2;
    new_d->type = type;
}

/* ===============================================
                    diff_last

    Retourne le dernière élément de la liste.
    ----------------------------------------------
    t_diff* list : liste à traiter
    ----------------------------------------------
   =============================================== */
t_diff *diff_last(t_diff* list) {

    if(list) {
        while(list->next) {
            list = list->next;
        }

        return list;
    } else
        return NULL;
}


/* ===============================================
                    diff_delete

    Vide et libère la liste list.
    ----------------------------------------------
    t_diff* list : liste à vider
    ----------------------------------------------
   =============================================== */
void diff_delete(t_diff* list) {

    t_diff* tmp = NULL;

    while(list) {
        tmp = list;
        list = list->next;
        free(tmp);
    }
}


/* ===============================================
             diff_display_end_of_file

    Si la fin du fichier indexé par f est atteinte
    affiche, affiche un saut de ligne ou le message
    "\ No newline at end of file" si show_msg est
    a 1.
    ----------------------------------------------
    u int end      : ligne
    t_index *f     : index
    s int show_msg : Si on doit afficher le message
    ----------------------------------------------
   =============================================== */
static void diff_display_end_of_file(unsigned int end, t_index *f, short int show_msg) {

    if(f->line_max > 0 && end == (f->line_max)-1) {
        fseek(f->f, 0, SEEK_END);
        if(getc(f->f) != '\n') { // Si on ne finit pas par un saut de ligne
            if(show_msg)
                puts("\\ No newline at end of file");
            else
                puts("");
        }
        line_go_to(f, f->line_max-1);
    }

}


/* ===============================================
             diff_display_file_name

    Permet d'afficher le nom du fichier indexé par
    f, ainsi qu'un timestamp.
    ----------------------------------------------
    t_index *f     : index
    ----------------------------------------------
   =============================================== */
static void diff_display_file_name(t_index *f) {

    int blank_lenght = 0;

    struct stat s;
    char stat_time[512] = {0};
    struct tm *stat_tm = NULL;

    stat(f->f_name, &s);
    stat_tm = localtime(&(s.st_mtime));
    strftime(stat_time, 512, "%Y-%m-%d %H:%M:%S", stat_tm);

    blank_lenght = 12-(strlen(f->f_name)%12);

    printf("%s", f->f_name);

    while(blank_lenght > 0) {
        fputc((int)' ', stdout);
        blank_lenght--;
    }

    printf("%s.%llu ", stat_time, (unsigned long long)s.st_mtime);

    strftime(stat_time, 512, "%z", stat_tm);
    printf("%s\n", stat_time);

}


/* ===============================================
             diff_display_file_name

    Permet d'afficher le nom de la fonction C
    présente à cette ligne.
    ----------------------------------------------
    t_index *f     : index
    u int line     : ligne actuelle
    ----------------------------------------------
   =============================================== */
static void diff_display_current_c_func(t_index* index, unsigned int line) {

    unsigned int i = 1;

    /* Si on a au moins une fonction C */
    if(index->c_func_nb > 0) {
        /* Si on est après la première */
        if(line > index->c_func[0]) {

            while(i < index->c_func_nb && index->c_func[i] < line)
                i++;

            lines_display_lenght(index, index->c_func[i-1], index->c_func[i-1], "", 40); // On affiche
        }
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
            diff_display_end_of_file(list_e->end_2, f2, 1);
        }
        else if(list_e->type == CHANGED_LINE){
            lines_display(f1, list_e->start_1, list_e->end_1, "< ");
            diff_display_end_of_file(list_e->end_1, f1, 1);

            fputs("---\n", stdout);
            lines_display(f2, list_e->start_2, list_e->end_2, "> ");
            diff_display_end_of_file(list_e->end_2, f2, 1);
        }
        else{
            lines_display(f1, list_e->start_1, list_e->end_1, "< ");
            diff_display_end_of_file(list_e->end_1, f1, 1);
        }


        /* Suite de la liste */
        list_e = list_e->next;
    }
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
    if(p->label == NULL)
        diff_display_file_name(f1);
    else
        puts(p->label);

    fputs("--- ",stdout);
    diff_display_file_name(f2);


    while(list_e) {

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

        /* Affichage de l'entête */
        fputs("***************", stdout);

        /* Si on doit afficher le nom de la fonction courrante */
        if(p->show_c_function) {
            fputc((int)' ', stdout);
            diff_display_current_c_func(f1, start_1);
        } else
            fputc((int)'\n', stdout);


        while( list_e->next && // Tant qu'on a un élement suivant & qu'il est assez prêt du précédent
             (((p->context > 0) && ((list_e->next->start_1 - list_e->end_1 - 1 < (2*p->context)) || (list_e->next->start_2 - list_e->end_2 - 1 < (2*p->context)))))) {

            list_e = list_e->next;
        }

        /* Fin */
        end_1 = list_e->end_1 + p->context >= (signed)(f1->line_max) ? (signed)(f1->line_max-1) : list_e->end_1 + p->context;
        end_2 = list_e->end_2 + p->context >= (signed)(f2->line_max) ? (signed)(f2->line_max-1) : list_e->end_2 + p->context;

        diff_display_context_lines(1, start_1, end_1, diff, f1, "*");
        diff_display_end_of_file(end_1, f1, 1);

        diff_display_context_lines(2, start_2, end_2, diff, f2, "-");
        diff_display_end_of_file(end_2, f2, 1);

        list_e = list_e->next;
    }
}

/* ===============================================
             diff_display_unified_lines

    Permet d'afficher les lignes au format unifié
    en respectant le contexte spécifié dans les
    paramètres.
    ----------------------------------------------
    t_index *f1       : Premier fichier
    int start_1       : Début dans premier fichier
    int end_1         : Fin dans premier fichier
    t_index *f2       : Second fichier
    int start_2       : Début dans second fichier
    int end_2         : Fin dans second fichier
    t_diff* diff      : liste des modifications
    ----------------------------------------------
   =============================================== */
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
        printf(" +0,0 @@");
    else {
        if((end_2+1) - (start_2+1) + 1 == 1)
             printf(" +%d @@", start_2+1);
        else
            printf(" +%d,%d @@", start_2+1, (end_2+1) - (start_2+1) + 1);
    }

    if(p->show_c_function) {
        fputc((int)' ', stdout);
        diff_display_current_c_func(f1, start_1);
    } else
        fputc((int)'\n', stdout);

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


/* ===============================================
             diff_display_unified

    Permet d'afficher les différences entre le
    premier fichier et le second fichier sous la
    forme unifiée.
    ----------------------------------------------
    t_diff* list_e    : liste des modifications
    t_index *f1       : Premier fichier
    t_index *f2       : Second fichier
    ----------------------------------------------
   =============================================== */
static void diff_display_unified(t_diff* list_e, t_index *f1, t_index *f2) {

    int start_1;
    int start_2;

    int end_1;
    int end_2;

    /* Affichage des noms de fichier */
    fputs("--- ",stdout);
    if(p->label == NULL)
        diff_display_file_name(f1);
    else
        puts(p->label);

    fputs("+++ ",stdout);
    diff_display_file_name(f2);

    t_diff *diff;

    while(list_e) {

        diff = list_e;

        /* Début */
        start_2 = (list_e->start_2 - p->context > 0) ? list_e->start_2 - p->context : 0 ;

        if(diff->type == ADDED_LINE) // Décalage d'ajout de lignes (Les lignes supprimées sont naturellements comptées dans le fichier 1)
                start_1 = ((list_e->start_1 + 1) - p->context > 0) ? (list_e->start_1 + 1) - p->context : 0 ;
        else
            start_1 = (list_e->start_1 - p->context > 0) ? list_e->start_1 - p->context : 0 ;

        while( list_e->next && // Tant qu'on a un élement suivant & qu'il est assez prêt du précédent
             ((p->context > 0) && ((list_e->next->start_1 - list_e->end_1 - 1 <= (2*p->context)) || (list_e->next->start_2 - list_e->end_2 - 1 <= (2*p->context))))) {

            list_e = list_e->next;
        }

        /* Fin */
        end_1 = list_e->end_1 + p->context >= (signed)(f1->line_max) ? (signed)(f1->line_max-1) : list_e->end_1 + p->context;

        if(diff->type == DELETED_LINE && p->context == 0) // Cas spécial
            end_2 = list_e->end_2 - 1 + p->context >= (signed)(f2->line_max) ? (signed)(f2->line_max-1) : list_e->end_2 - 1 + p->context;
        else
            end_2 = list_e->end_2 + p->context >= (signed)(f2->line_max) ? (signed)(f2->line_max-1) : list_e->end_2 + p->context;

        diff_display_unified_lines(f1, start_1, end_1, f2, start_2, end_2, diff);
        diff_display_end_of_file(end_1, f1, 1);

        list_e = list_e->next;
    }

}


/* ===============================================
             diff_display_columns_common

    Permet d'afficher les lignes communes sous
    formes de colonnes.
    ----------------------------------------------
    int start         : Premier ligne à afficher
    u int lenght      : Nombre de lignes à afficher
    t_index *f1       : Premier fichier
    t_index *f2       : Second fichier
    u int char_ligne  : Largeur d'une ligne
    u int char_center : Largeur centre
    ----------------------------------------------
   =============================================== */
static void diff_display_columns_common(int start_1, int start_2, unsigned int length, t_index *f1, t_index *f2, unsigned int char_ligne, unsigned int char_center, char op) {

    unsigned int i = 0, j = 0;
    char tmp = 0;

    if(op != ' ' || (op == ' ' && !(p->suppress_common_lines))) {
        /* Pour chaque lignes */
        for(i = 0; i < length ; i++) {
            /* On va sur la ligne correspondante */
            line_go_to(f1, i + start_1);
            line_go_to(f2, i + start_2);

            /* Ligne de f1 */
            for(j = 0; j < char_ligne; j++){
                /* On affiche jusqu'à la fin de ligne */
                if(!(f1->lines) && (tmp = fgetc(f1->f)) != '\n' && tmp != EOF)
                    putchar((int)tmp);
                else if(f1->lines && f1->lines[f1->line][j] != '\n' && f1->lines[f1->line][j] != '\0')
                    putchar((int)f1->lines[f1->line][j]);
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

            if(op == ' ') {
                if(p->left_column)
                    putchar((int)'(');
                else
                    putchar((int)op);
            } else
                putchar((int)op);

            for(j = 0; j < ((char_center-1)/2) + ((char_center-1)%2); j++)
                putchar((int)' ');

            j = 0;
            /* Ligne de f2 */
            if(op != ' ' || (op == ' ' && !(p->left_column))) {
                for(; j < char_ligne; j++){
                    /* On affiche jusqu'à la fin de ligne */
                    if(!(f2->lines) && (tmp = fgetc(f2->f)) != '\n' && tmp != EOF)
                        putchar((int)tmp);
                    else if(f2->lines && f2->lines[f2->line][j] != '\n' && f2->lines[f2->line][j] != '\0')
                        putchar((int)f2->lines[f2->line][j]);
                    else
                        break;
                }
            }
            /* Ce qui manque en espace */
            for(; j < char_ligne; j++){
                putchar((int)' ');
            }

            putchar((int)'\n');
        }
    }
}


/* ===============================================
             diff_display_columns_added

    Permet d'afficher les lignes ajoutées sous
    formes de colonnes.
    ----------------------------------------------
    int start         : Premier ligne à afficher
    u int lenght      : Nombre de lignes à afficher
    t_index *f        : Second fichier
    u int char_ligne  : Largeur d'une ligne
    u int char_center : Largeur centre
    ----------------------------------------------
   =============================================== */
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
            if(!(f->lines) && (tmp = fgetc(f->f)) != '\n' && tmp != EOF)
                putchar((int)tmp);
            else if(f->lines && f->lines[f->line][j] != '\n' && f->lines[f->line][j] != '\0')
                putchar((int)f->lines[f->line][j]);
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


/* ===============================================
             diff_display_columns_deleted

    Permet d'afficher les lignes supprimées sous
    formes de colonnes.
    ----------------------------------------------
    int start         : Premier ligne à afficher
    u int lenght      : Nombre de lignes à afficher
    t_index *f        : Premier fichier
    u int char_ligne  : Largeur d'une ligne
    u int char_center : Largeur centre
    ----------------------------------------------
   =============================================== */
static void diff_display_columns_deleted(int start, unsigned int lenght, t_index *f, unsigned int char_ligne, unsigned int char_center) {

    unsigned int i = 0, j = 0;
    char tmp = 0;

    for(i = 0; i < lenght; i++) {
        line_go_to(f, i+start);

        /* Ligne supprimmées */
        for(j = 0; j < char_ligne; j++) {
            if(!(f->lines) && (tmp = fgetc(f->f)) != '\n' && tmp != EOF)
                putchar((int)tmp);
            else if(f->lines && f->lines[f->line][j] != '\n' && f->lines[f->line][j] != '\0')
                putchar((int)f->lines[f->line][j]);
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


/* ===============================================
                    diff_display_columns

    Permet d'afficher la liste des modification pour
    passer du premier au second fichier sous la forme
    de colonnes comparatives.
    ----------------------------------------------
    t_diff* list_e    : liste des modifications
    t_index *f1       : index du premier fichier
    t_index *f2       : index du second fichier
    int show_max_char : largeur colones maximum
    ----------------------------------------------
   =============================================== */
static void diff_display_columns(t_diff* list_e, t_index *f1, t_index *f2, int show_max_char) {

    unsigned int char_ligne = (show_max_char - 5) >= 0 ? (show_max_char - 3) : 0; // Nombre de colonnes
    unsigned int char_center = 3 + char_ligne%2; // Nombre de colone centrale

    /* On s'assure d'être revenu au début */
    line_go_to(f1, 0);
    line_go_to(f2, 0);

    if(list_e) {
        /* Tant qu'il reste un élément */
        while(list_e) {

            if(list_e->start_1 - f1->line > 0) // Si des lignes avant le prochain diff, on affiche
                diff_display_columns_common(f1->line, f2->line, list_e->start_1 - f1->line, f1, f2, char_ligne/2, char_center, ' ');

            if(list_e->type == ADDED_LINE) // Si des lignes ajoutées
                diff_display_columns_added(list_e->start_2, list_e->end_2 - list_e->start_2 + 1, f2, char_ligne/2, char_center);

            else if(list_e->type == DELETED_LINE) // Si des lignes supprimmées
                diff_display_columns_deleted(list_e->start_1, list_e->end_1 - list_e->start_1 + 1, f1, char_ligne/2, char_center);

            else { // Si des lignes modifiées

                /* Si plus de ligne dans f1 que dans f2 */
                if(list_e->end_1 - list_e->start_1 > list_e->end_2 - list_e->start_2) {
                    diff_display_columns_common(list_e->start_1,  list_e->start_2, list_e->end_2 - list_e->start_2+1, f1, f2, char_ligne/2, char_center, '|');
                    diff_display_columns_deleted(list_e->start_1 + (list_e->end_1 - list_e->start_1), (list_e->end_1 - list_e->start_1)-(list_e->end_2 - list_e->start_2), f1, char_ligne/2, char_center);
                }
                /* Moins ou autant de ligne dans f1 que dans f2 */
                else {
                    diff_display_columns_common(list_e->start_1,  list_e->start_2, list_e->end_1 - list_e->start_1+1, f1, f2, char_ligne/2, char_center, '|');
                    diff_display_columns_added(f2->line + 1, (list_e->end_2 - list_e->start_2)-(list_e->end_1 - list_e->start_1), f2, char_ligne/2, char_center);
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


/* ===============================================
             diff_display_ed_get_diff

    Permet de parcourir la liste des différences
    suivant un indice.
    ----------------------------------------------
    t_diff* list_e : liste des modifications
    u int n        : indice
    ----------------------------------------------
   =============================================== */
static t_diff* diff_display_ed_get_diff(t_diff* list_e, unsigned int n) {

    while(n > 0) {
        n--;
        list_e = list_e->next;
    }

    return list_e;
}


/* ===============================================
                    diff_display_ed

    Permet d'afficher la liste des modification pour
    passer du premier au second fichier sous la forme
    d'un edit_script.
    ----------------------------------------------
    t_diff* list_e : liste des modifications
    t_index *f    : index du second fichier
    ----------------------------------------------
   =============================================== */
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
            diff_display_end_of_file(tmp->end_2, f, 0);

            puts(".");
        } else if(tmp->type == CHANGED_LINE) {
            if(tmp->end_1 - tmp->start_1 + 1 == 1)
                printf("%dc\n", tmp->start_1+1);
            else
                printf("%d,%dc\n", tmp->start_1+1, tmp->end_1 - tmp->start_1 + 1);

            lines_display(f,tmp->start_2, tmp->end_2, "");
            diff_display_end_of_file(tmp->end_2, f, 0);

            puts(".");
        } else {
            if(tmp->end_1 - tmp->start_1 + 1 == 1)
                printf("%dd\n", tmp->start_1+1);
            else
                printf("%d,%dd\n", tmp->start_1+1, tmp->end_1 - tmp->start_1 + 1);
        }

    }
}

/* ===============================================
                      diff_display

    Permet d'afficher la liste des modification pour
    passer du fichier indexé par f1 au fichier indexé
    par f2 en utilisant le style et les options
    défini par les paramètres.
    ----------------------------------------------
    t_diff* list_e : liste des modifications
    t_index *f1    : index du premier fichier
    t_index *f2    : index du second fichier
    ----------------------------------------------
   =============================================== */
void diff_display(t_diff* list_e, t_index *f1, t_index *f2) {

    #ifdef DEBUG
        printf("Start of the result display...\n");
    #endif

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

    #ifdef DEBUG
        printf("...display completed\n--------------\n");
    #endif

}
