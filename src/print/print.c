#include "print.h"

/* Statiques */
static void print_diff_regular(File files[]);
static void print_space(uint n);
static void print_lines(File *file, ulint start, ulint end, const char *line_start, _bool show_end_of_file);
static void print_lines_length(File *file, ulint start, ulint end, const char *line_start, ulint length, _bool show_end_of_file);
static void print_current_c_func(File *file, ulint line);
static void print_file_name(struct stat *st, char* name);
static void print_diff_context(File files[]);
static void print_diff_context_lines(const char* line_start, File* file, ulint start, ulint end, _bool show_line);
static void print_diff_unified(File files[]);
static void print_diff_unified_lines(File files[], ulint start_1, ulint end_1, ulint start_2, ulint end_2);
static void print_diff_ed(File files[]);

/* ===============================================
                  lines_display

    Affiche les lignes de f de start à end (compris)
    en ajoutant line_start avant chaque ligne.
    Saute une ligne après chaque ligne écrite.
    ----------------------------------------------
    t_index *f       : fichier où sont les lignes.
    u int start      : première ligne.
    u int end        : dernière ligne.
    char* line_start : à afficher avant chaque ligne
   =============================================== */
static void print_lines(File *file, ulint start, ulint end, const char *line_start, _bool show_end_of_file) {
    print_lines_length(file, start, end, line_start, 0, show_end_of_file);
}

/* ===============================================
                lines_display_length

    Affiche les lignes de f de start à end (compris)
    en ajoutant line_start avant chaque ligne avec
    un maximum de length caractères. 0 permet de ne
    pas utiliser de limitation.
    Saute une ligne après chaque ligne écrite.
    ----------------------------------------------
    t_index *f       : fichier où sont les lignes.
    u int start      : première ligne.
    u int end        : dernière ligne.
    char* line_start : à afficher avant chaque ligne
    u int length     : nombre de char max
   =============================================== */
static void print_lines_length(File *file, ulint start, ulint end, const char *line_start, ulint length, _bool show_end_of_file) {

    ulint i = 0, j = 0, l = 0;
    int c = 0;


    if(start <= end && end <= file->i->line_max) { // Si taille possible
        for(i = start; i <= end; i++) { // Pour chacune des lignes à afficher

            if(line_start)
                fputs(line_start, stdout);

            if(file->i->lines[i].length > 0 ) {
                l = (length > 0 && length <= file->i->lines[i].length ? length : file->i->lines[i].length);

                #ifdef _WIN32
                fseeko64(file->f, (long long)file->i->lines[i].start, SEEK_SET);
                #else
                fseeko(file->f, (long long)file->i->lines[i].start, SEEK_SET);
                #endif
                for(j = 0; j < l; j++) {
                    if((c = getc(file->f)) == '\t')
                        print_space(p->size_tab);
                    else
                        putchar(c);
                }
            }

            putchar('\n');
        }

        if(show_end_of_file && length == 0 && i == file->i->line_max && file->i->lines[i-1].end_line == END_OF_FILE)
            puts("\\ No newline at end of file");
    }
}

static void print_current_c_func(File *file, ulint line) {

    ulint i = 1, c_func = 0;

    while(i < line) {
        if(file->i->lines[i].is_c_func)
            c_func = i;
        i++;
    }

    if(c_func > 0 || (line == 0 && file->i->line_max >= 1 && file->i->lines[0].is_c_func))
        print_lines_length(file, c_func, c_func, "", 40, _false);

}


/* ===============================================
             diff_display_file_name

    Permet d'afficher le nom du fichier indexé par
    f, ainsi qu'un timestamp.
    ----------------------------------------------
    t_index *f     : index
    ----------------------------------------------
   =============================================== */
static void print_file_name(struct stat *st, char* name) {

    char stat_time[512] = {0};
    struct tm *stat_tm = NULL;

    stat_tm = localtime(&(st->st_mtime));
    strftime(stat_time, 512, "%Y-%m-%d %H:%M:%S", stat_tm);

    printf("%s", name);
    print_space(12-(strlen(name)%12));

    #ifdef _WIN32
        printf("%s.%I64u ", stat_time, (unsigned long long)st->st_mtime);
    #else
        printf("%s.%llu ", stat_time, (unsigned long long)st->st_mtime);
    #endif

    strftime(stat_time, 512, "%z", stat_tm);
    printf("%s\n", stat_time);
}


/* ===============================================
                    print_space

    Permet d'afficher simplement un nombre donné
    d'espace sur la sortie standard
    ----------------------------------------------
    u int n : nombre d'espaces
   =============================================== */
static void print_space(uint n) {
    uint i = 0;

    for(i = 0; i < n; i++)
        fputc(' ', stdout);
}


void print_diff(File files[]) {

    #ifdef DEBUG
        START_TIMER;
        printf("Start of the result display...\n");
    #endif

    /* Format de sortie contextuel */
    if(p->o_style == CONTEXT)
        print_diff_context(files);
    /* Format de sortie unifé */
    else if(p->o_style == UNIFIED)
        print_diff_unified(files);
    /* Format edit script */
    else if(p->o_style == EDIT_SCRIPT)
        print_diff_ed(files);
    /* Format en colonnes */
    else if(p->o_style == COLUMNS)
        //diff_display_columns(diff, f1, f2, p->show_max_char);
        puts("A faire");
    /* Format pas défaut */
    else
        print_diff_regular(files);

    #ifdef DEBUG
        STOP_TIMER;
        printf("...display completed (%.4fs) \n--------------\n", GET_TIMER_VALUE);
    #endif

}

static void print_diff_regular(File files[]) {

    ulint i = 0, j = 0;
    ulint length_1 = 0, length_2 = 0;

    /* Pour chaque ligne */
    /* Fonctionne car si les lignes ajoutées sont après la fin, la dernière ligne de l'autre fichier sera modifiée */
    for(; i < files[0].i->line_max && j < files[1].i->line_max; i++, j++) {

        /* Une des deux lignes modifiée */
        if(files[0].i->lines[i].modified || files[1].i->lines[j].modified) {

            /* Lignes supprimées */
            if(files[0].i->lines[i].modified == LINE_DEL) {
                if((length_1 = diff_get_length(files[0].i, i)) == 1)
                    printf("%"SHOW_ulint"d%"SHOW_ulint"\n", i+1, j);
                else
                    printf("%"SHOW_ulint",%"SHOW_ulint"d%"SHOW_ulint"\n", i+1, length_1 + i, j);

                print_lines(&files[0], i, length_1 + i - 1, "< ", _true);
            }
            /* Lignes ajoutées */
            else if(files[1].i->lines[j].modified == LINE_ADD) {
                if((length_2 = diff_get_length(files[1].i, j)) == 1)
                    printf("%"SHOW_ulint"a%"SHOW_ulint"\n",i , j+1);
                else
                    printf("%"SHOW_ulint"a%"SHOW_ulint",%"SHOW_ulint"\n",i,  j+1, length_2 + j);

                print_lines(&files[1], j, length_2 + j - 1, "> ", _true);
            }
            /* Lignes modifiées */
            else {
                if((length_1 = diff_get_length(files[0].i, i)) == 1)
                    printf("%"SHOW_ulint"c", i+1);
                else
                    printf("%"SHOW_ulint",%"SHOW_ulint"c", i+1, length_1 + i);

                if((length_2 = diff_get_length(files[1].i, j)) == 1)
                    printf("%"SHOW_ulint"\n", j+1);
                else
                    printf("%"SHOW_ulint",%"SHOW_ulint"\n", j+1, length_2 + j);

                print_lines(&files[0], i, length_1 + i - 1, "< ", _true);
                fputs("---\n", stdout);
                print_lines(&files[1], j, length_2 + j - 1, "> ", _true);
            }

            /* On reprend après */
            i += length_1;
            j += length_2;
        }
    }
}


static void print_diff_context_lines(const char* line_start, File* file, ulint start, ulint end, _bool show_line) {

    ulint i = start;
    fputs(line_start, stdout);

    if(end == start)
        printf(" %"SHOW_ulint" ", start);
    else
        printf(" %"SHOW_ulint",%"SHOW_ulint" ", start, end);

    puts(line_start);

    if(show_line) {
        for(; i <= end; i++){ // Sale à refaire
            if(file->i->lines[i-1].modified == LINE_ADD)
                print_lines(file, i-1, i-1, "+ ", _true);
            else if(file->i->lines[i-1].modified == LINE_DEL)
                print_lines(file, i-1, i-1, "- ", _true);
            else if(file->i->lines[i-1].modified == LINE_CHANGE)
                print_lines(file, i-1, i-1, "! ", _true);
            else
                print_lines(file, i-1, i-1, "  ", _true);
        }
    }
}


static void print_diff_context(File files[]) {

    /* Attention, range < aux autres affichages */
    ulint i = 1, j = 1;
    ulint start_1 = 0, start_2 = 0, end_1 = 0, end_2 = 0;

    _bool is_modified_1  =_false, is_modified_2  =_false;

    int c = 0;

    fputs("*** ",stdout), print_file_name(&files[0].st, files[0].label);
    fputs("--- ",stdout), print_file_name(&files[1].st, files[1].label);

    /* Pour chaque ligne */
    for(; i <= files[0].i->line_max && j <= files[1].i->line_max; i++, j++) {

        /* Une des deux lignes est modifiée */
        if(files[0].i->lines[i-1].modified || files[1].i->lines[j-1].modified) {
            is_modified_1 = is_modified_2 = _false;

            c = 2 * p->context;

            end_1 = start_1 = ( p->context < i ? i - p->context : 1 );
            end_2 = start_2 = ( p->context < j ? j - p->context : 1 );

            /* Tant qu'on peut afficher du contexte, on avance */
            while(c >= 0) {
                /* Si on arrive pas encore à la fin */
                if(i <= files[0].i->line_max && j <= files[1].i->line_max) {

                    if(files[0].i->lines[i-1].modified || files[1].i->lines[j-1].modified) {

                        c = 2 * p->context - 1;

                        if(files[0].i->lines[i-1].modified) {
                            is_modified_1 = _true;
                            i = i + diff_get_length(files[0].i, i-1);
                        }

                        if(files[1].i->lines[j-1].modified) {
                            is_modified_2 = _true;
                            j = j + diff_get_length(files[1].i, j-1);
                        }

                        end_1 = i + p->context - 1;
                        end_2 = j + p->context - 1;

                    }else /* On avance en diminuant le context */
                        c--;
                    j++, i++;
                } else
                    break;
            }

            /* Si on dépasse la fin, on revient */
            if(end_1 >= files[0].i->line_max + 1)
                end_1 = files[0].i->line_max;

            if(end_2 >= files[1].i->line_max + 1)
                end_2 = files[1].i->line_max;

            /* Cas spécial */
            if(p->context == 0 && !is_modified_1)
                start_1--;

            else if(p->context == 0 && !is_modified_2)
                start_2--;

            fputs("***************", stdout);

            /* Si on doit afficher le nom de la fonction courrante */
            if(p->show_c_function) {
                fputc(' ', stdout);
                print_current_c_func(&files[0], start_1-1);
            } else
                fputc('\n', stdout);

            print_diff_context_lines("***", &files[0], start_1, end_1, is_modified_1);
            print_diff_context_lines("---", &files[1], start_2, end_2, is_modified_2);
        }
    }
}

static void print_diff_unified_lines(File files[], ulint start_1, ulint end_1, ulint start_2, ulint end_2) {

    ulint i = start_1, j = start_2, start_print = 0;

    /* Affichage numéro de ligne */
    fputs("@@ -", stdout);
    if(end_1 - start_1 == 1)
        printf("%"SHOW_ulint"", start_1);
    else
        printf("%"SHOW_ulint",%"SHOW_ulint"", start_1, end_1-start_1);

    if(end_2 - start_2 == 1)
        printf(" +%"SHOW_ulint" @@", start_2);
    else
        printf(" +%"SHOW_ulint",%"SHOW_ulint" @@", start_2, end_2-start_2);

    if(p->show_c_function) {
        fputc(' ', stdout);
        print_current_c_func(&files[0], start_1-1);
    } else
        fputc('\n', stdout);

    /* Affichage des lignes */
    while(i < end_1) {
        /* Si lignes différentes */
        if(files[0].i->lines[i-1].modified || files[1].i->lines[j-1].modified) {
            /* Lignes supprimées */
            if(files[0].i->lines[i-1].modified) {
                start_print = i;
                do {
                    i++;
                }while(files[0].i->lines[i-1].modified
                       && i <= files[0].i->line_max
                       && i < end_1);

                print_lines(&files[0], start_print-1, i-2, "- ", _true);
            }
            /* Lignes ajoutées */
            if (files[1].i->lines[i-1].modified) {
                start_print = j;
                do {
                    j++;
                }while(files[1].i->lines[j-1].modified
                       && j <= files[1].i->line_max
                       && j < end_2);

                print_lines(&files[1], start_print-1, j-2, "+ ", _true);
            }

        } else {
            /* Lignes identiques */
            start_print = i;
            do {
                i++;
                j++;
            }while(!(files[0].i->lines[i-1].modified || files[1].i->lines[j-1].modified)
                   && i <= files[0].i->line_max && j <= files[1].i->line_max
                   && i < end_1 && j < end_2 );

            print_lines(&files[0], start_print-1, i-2, "  ", _true);
        }
    }
}

static void print_diff_unified(File files[]) {

    ulint i = 1, j = 1;
    ulint start_1 = 0, start_2 = 0, end_1 = 0, end_2 = 0;
    _bool is_modified_1  =_false, is_modified_2  =_false;

    int c = 0;

    fputs("--- ",stdout), print_file_name(&files[0].st, files[0].label);
    fputs("+++ ",stdout), print_file_name(&files[1].st, files[1].label);

    /* Pour chaque ligne */
    for(; i <= files[0].i->line_max && j <= files[1].i->line_max; i++, j++) {
        /* Si différence */
        if(files[0].i->lines[i-1].modified || files[1].i->lines[j-1].modified) {
            c = 2 * p->context;

            is_modified_1 = _false, is_modified_2 = _false;

            end_1 = start_1 = ( p->context < i ? i - p->context : 1 );
            end_2 = start_2 = ( p->context < j ? j - p->context : 1 );

            while(c >= 0) {
                if(i <= files[0].i->line_max && j <= files[1].i->line_max) {
                    if(files[0].i->lines[i-1].modified || files[1].i->lines[j-1].modified) {
                        c = 2 * p->context - 1;

                        if(files[0].i->lines[i-1].modified) {
                            i = i + diff_get_length(files[0].i, i-1);
                            is_modified_1 = _true;
                        }

                        if(files[1].i->lines[j-1].modified) {
                            j = j + diff_get_length(files[1].i, j-1);
                            is_modified_2 = _true;
                        }

                        end_1 = i + p->context;
                        end_2 = j + p->context;

                    } else
                        c--;
                    i++, j++;
                } else
                    break;
            }

            /* Si on dépasse la fin, on revient */
            if(end_1 > files[0].i->line_max+1)
                end_1 = files[0].i->line_max+1;

            if(end_2 > files[1].i->line_max+1)
                end_2 = files[1].i->line_max+1;

            /* Cas spécial */
            if(p->context == 0 && !is_modified_1) {
                start_1--;
                end_1 = start_1;
            }
            else if(p->context == 0 && !is_modified_2) {
                start_2--;
                end_2 = start_2;
            }

            print_diff_unified_lines(files, start_1, end_1, start_2, end_2);
        }
    }
}


static void print_diff_ed(File files[]) {

    ulint i = files[0].i->line_max, j = files[1].i->line_max;
    ulint end_1 = 0, end_2 = 0;

    for(; i > 0 && j > 0; i--, j--) {
        if(files[0].i->lines[i-1].modified == LINE_DEL) {
            end_1 = i;

            do {
                i--;
            }while(files[0].i->lines[i-1].modified == LINE_DEL);

            i++;

            if(i == end_1)
                printf("%"SHOW_ulint"d\n", i);
            else
                printf("%"SHOW_ulint",%"SHOW_ulint"d\n", i, end_1-i);

        } else if(files[1].i->lines[j-1].modified == LINE_ADD) {
            end_2 = j;

            do {
                j--;
            }while(files[1].i->lines[j-1].modified == LINE_ADD);

            j++;

            printf("%"SHOW_ulint"a\n", i);
            print_lines(&files[1], j-1, end_2-1, "", _false);
            puts(".");

        } else if(files[0].i->lines[i-1].modified == LINE_CHANGE || files[1].i->lines[j-1].modified == LINE_CHANGE) {

            end_1 = i;
            end_2 = j;

            do {
                j--;
            }while(files[1].i->lines[j-1].modified == LINE_CHANGE);

            j++;

            do {
                i--;
            }while(files[0].i->lines[i-1].modified == LINE_CHANGE);

            i++;

            if(i == end_1)
                printf("%"SHOW_ulint"c\n", i);
            else
                printf("%"SHOW_ulint",%"SHOW_ulint"c\n", i, end_1 - i);

            print_lines(&files[1], j-1, end_2-1, "", _false);
            puts(".");
        }
    }

    if(files[0].i->lines[files[0].i->line_max-1].end_line == END_OF_FILE)
        printf("diff: No newline at end of file %s\n", files[0].label);

    if(files[1].i->lines[files[1].i->line_max-1].end_line == END_OF_FILE)
        printf("diff: No newline at end of file %s\n", files[1].label);

}
