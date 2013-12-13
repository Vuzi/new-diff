#include "print.h"

/* Statiques */
static void print_diff_regular(File files[]);
static void print_space(uint n);
static void print_lines(File *file, ulint start, ulint end, const char *line_start, _bool show_end_of_file);
static void print_lines_length(File *file, ulint start, ulint end, const char *line_start, ulint length, _bool show_end_of_file);
static void print_current_func(File *file, ulint line);
static void print_file_name(struct stat *st, char* name);
static void print_diff_context(File files[]);
static void print_diff_context_lines(const char* line_start, File* file, ulint start, ulint end, _bool show_line);
static void print_diff_unified(File files[]);
static void print_diff_unified_lines(File files[], ulint start_1, ulint end_1, ulint start_2, ulint end_2);
static void print_diff_ed(File files[]);
static void print_diff_rcs(File files[]);
static void print_diff_columns(File files[]);
static void print_diff_ifdef(File files[]);

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


    if(start <= end && end <= file->i->line_max && !file->empty) { // Check size
        for(i = start; i <= end; i++) { // For each line to print

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
                    c = getc(file->f);
                    if(p->expand_tab && c == '\t')
                        print_space(p->size_tab);
                    else
                        putchar(c);
                }
            }

            if(length == 0)
                putchar('\n');
        }

        if(show_end_of_file && length == 0 && i == file->i->line_max && file->i->lines[i-1].end_line == END_OF_FILE)
            puts("\\ No newline at end of file");
    }
}

static void print_current_func(File *file, ulint line) {

    ulint i = 1, func = 0;

    while(i <= line) {
        if(file->i->lines[i].is_func)
            func = i;
        i++;
    }

    if(func > 0 || (line == 0 && file->i->line_max >= 1 && file->i->lines[0].is_func))
        print_lines_length(file, func, func, "", 40, _false);

    fputc((int)'\n', stdout);
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
    print_space(12-(diff_strlen(name)%12));

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

    if(p->o_style == CONTEXT)
        print_diff_context(files);
    else if(p->o_style == UNIFIED)
        print_diff_unified(files);
    else if(p->o_style == EDIT_SCRIPT)
        print_diff_ed(files);
    else if(p->o_style == COLUMNS)
        print_diff_columns(files);
    else if(p->o_style == RCS)
        print_diff_rcs(files);
    else if(p->o_style == IFDEF)
        print_diff_ifdef(files);
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

    // For each line
    for(; i < files[0].i->line_max && j < files[1].i->line_max; i++, j++) {

        // One of them is changed
        if(files[0].i->lines[i].modified || files[1].i->lines[j].modified) {

            length_1 = 0;
            length_2 = 0;

            // Deleted
            if(files[0].i->lines[i].modified == LINE_DEL) {
                if((length_1 = diff_get_length(files[0].i, i)) == 1)
                    printf("%"SHOW_ulint"d%"SHOW_ulint"\n", i+1, j);
                else
                    printf("%"SHOW_ulint",%"SHOW_ulint"d%"SHOW_ulint"\n", i+1, length_1 + i, j);

                print_lines(&files[0], i, length_1 + i - 1, "< ", _true);
            }
            // Added
            else if(files[1].i->lines[j].modified == LINE_ADD) {
                if((length_2 = diff_get_length(files[1].i, j)) == 1)
                    printf("%"SHOW_ulint"a%"SHOW_ulint"\n",i , j+1);
                else
                    printf("%"SHOW_ulint"a%"SHOW_ulint",%"SHOW_ulint"\n",i,  j+1, length_2 + j);

                print_lines(&files[1], j, length_2 + j - 1, "> ", _true);
            }
            // Changed
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

            i += length_1-1;
            j += length_2-1;
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
        for(; i <= end; i++){
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

    ulint i = 1, j = 1;
    ulint start_1 = 0, start_2 = 0, end_1 = 0, end_2 = 0;

    _bool is_modified_1  =_false, is_modified_2  =_false;

    int c = 0;

    fputs("*** ",stdout), print_file_name(&files[0].st, files[0].label);
    fputs("--- ",stdout), print_file_name(&files[1].st, files[1].label);

    // For each line
    for(; i <= files[0].i->line_max && j <= files[1].i->line_max; i++, j++) {

        // One of them is changed
        if(files[0].i->lines[i-1].modified || files[1].i->lines[j-1].modified) {
            is_modified_1 = is_modified_2 = _false;

            c = 2 * p->context;

            end_1 = start_1 = ( p->context < i ? i - p->context : 1 );
            end_2 = start_2 = ( p->context < j ? j - p->context : 1 );

            // Don't stop if we can show context lines
            while(c >= 0) {
                // If we aren't at the end
                if(i <= files[0].i->line_max && j <= files[1].i->line_max) {

                    if(files[0].i->lines[i-1].modified || files[1].i->lines[j-1].modified) {

                        c = 2 * p->context - 1; // context back to start

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

                    }else // Lower context
                        c--;
                    j++, i++;
                } else
                    break;
            }

            // If we're too far, go back
            if(end_1 >= files[0].i->line_max + 1)
                end_1 = files[0].i->line_max;

            if(end_2 >= files[1].i->line_max + 1)
                end_2 = files[1].i->line_max;

            // Special case
            if(p->context == 0 && !is_modified_1)
                start_1--;

            else if(p->context == 0 && !is_modified_2)
                start_2--;

            fputs("***************", stdout);

            // If we should print the current function name
            if(p->show_regex_function) {
                fputc(' ', stdout);
                print_current_func(&files[0], start_1-1);
            } else
                fputc('\n', stdout);

            print_diff_context_lines("***", &files[0], start_1, end_1, is_modified_1);
            print_diff_context_lines("---", &files[1], start_2, end_2, is_modified_2);
        }
    }
}

static void print_diff_unified_lines(File files[], ulint start_1, ulint end_1, ulint start_2, ulint end_2) {

    ulint i = start_1, j = start_2, start_print = 0;

    // Line number
    fputs("@@ -", stdout);
    if(end_1 - start_1 == 1)
        printf("%"SHOW_ulint"", start_1);
    else
        printf("%"SHOW_ulint",%"SHOW_ulint"", start_1, end_1-start_1);

    if(end_2 - start_2 == 1)
        printf(" +%"SHOW_ulint" @@", start_2);
    else
        printf(" +%"SHOW_ulint",%"SHOW_ulint" @@", start_2, end_2-start_2);

    if(p->show_regex_function) {
        fputc(' ', stdout);
        print_current_func(&files[0], start_1-1);
    } else
        fputc('\n', stdout);

    // Lines
    while(i < end_1) {
        // Modified
        if(files[0].i->lines[i-1].modified || files[1].i->lines[j-1].modified) {
            // Deleted
            if(files[0].i->lines[i-1].modified) {
                start_print = i;
                do {
                    i++;
                }while(files[0].i->lines[i-1].modified
                       && i <= files[0].i->line_max
                       && i < end_1);

                print_lines(&files[0], start_print-1, i-2, "- ", _true);
            }
            // Added
            if (files[1].i->lines[j-1].modified) {
                start_print = j;
                do {
                    j++;
                }while(files[1].i->lines[j-1].modified
                       && j <= files[1].i->line_max
                       && j < end_2);

                print_lines(&files[1], start_print-1, j-2, "+ ", _true);
            }
        } else {
            // Identical
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

    // For each line
    for(; i <= files[0].i->line_max && j <= files[1].i->line_max; i++, j++) {
        // Different
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

            // If we're too far, go back
            if(end_1 > files[0].i->line_max+1)
                end_1 = files[0].i->line_max+1;

            if(end_2 > files[1].i->line_max+1)
                end_2 = files[1].i->line_max+1;

            // Special case
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


static void print_diff_rcs(File files[]) {

    ulint i = 0, j = 0;
    ulint end_1 = 0, end_2 = 0;

    for(; i < files[0].i->line_max && j < files[1].i->line_max; i++, j++) {
        if(files[0].i->lines[i].modified == LINE_DEL) {
            end_1 = i;

            do {
                i++;
            }while(files[0].i->lines[i].modified == LINE_DEL);

            i--;

            printf("d%"SHOW_ulint" %"SHOW_ulint"\n", i+1, end_1-i+1);

        } else if(files[1].i->lines[j].modified == LINE_ADD) {
            end_2 = j;

            do {
                j++;
            }while(files[1].i->lines[j].modified == LINE_ADD);

            j--;

            printf("a%"SHOW_ulint" %"SHOW_ulint"\n", j+1, end_2-j+1);
            print_lines(&files[1], j, end_2, "", _false);

        } else if(files[0].i->lines[i].modified == LINE_CHANGE || files[1].i->lines[j].modified == LINE_CHANGE) {

            end_1 = i;
            end_2 = j;

            do {
                j++;
            }while(files[1].i->lines[j].modified == LINE_CHANGE);

            j--;

            do {
                i++;
            }while(files[0].i->lines[i].modified == LINE_CHANGE);

            i--;

            printf("d%"SHOW_ulint" %"SHOW_ulint"\n", i+1, end_1-i+1);
            printf("a%"SHOW_ulint" %"SHOW_ulint"\n", j+1, end_2-j+1);
            print_lines(&files[1], j, end_2, "", _false);
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


static void print_diff_columns(File files[]) {

    uint char_ligne = (p->width >= 5) ? (p->width - 3) : 0; // Number of columns
    uint char_center = 3 + char_ligne%2; // Size of center column
    char_ligne = char_ligne / 2;

    ulint i = 0, j = 0;

    for(; i < files[0].i->line_max && j < files[1].i->line_max;) {

        if(files[1].i->lines[j].modified == LINE_ADD) {
            print_space(char_center/2 + char_ligne);
            fputc((int)'>', stdout);
            print_space(char_center/2);

            if(char_ligne > 0) {
                print_lines_length(&files[1], j, j, "", char_ligne, _false);
                if(files[1].i->lines[j].length < char_ligne)
                    print_space(char_ligne - files[1].i->lines[j].length);
            }

            fputc((int)'\n', stdout);
            j++;
        } else if(files[0].i->lines[i].modified == LINE_DEL) {
            if(char_ligne > 0) {
                print_lines_length(&files[0], i, i, "", char_ligne, _false);
                if(files[1].i->lines[j].length < char_ligne)
                    print_space(char_ligne - files[1].i->lines[j].length);
            }

            print_space(char_center/2);
            fputc((int)'<', stdout);
            print_space(char_center/2 + char_ligne);
            fputc((int)'\n', stdout);
            i++;
        } else if(files[0].i->lines[i].modified == LINE_CHANGE) {
            if(char_ligne > 0) {
                print_lines_length(&files[0], i, i, "", char_ligne, _false);
                if(files[1].i->lines[j].length < char_ligne)
                    print_space(char_ligne - files[1].i->lines[j].length);
            }

            print_space(char_center/2);
            fputc((int)'|', stdout);
            print_space(char_center/2);

            if(char_ligne > 0) {
                print_lines_length(&files[1], j, j, "", char_ligne, _false);
                if(files[1].i->lines[j].length < char_ligne)
                    print_space(char_ligne - files[1].i->lines[j].length);
            }

            fputc((int)'\n', stdout);
            i++;
            j++;
        } else {
            if(!(p->suppress_common_lines)) {
                if(char_ligne > 0) {
                    print_lines_length(&files[0], i, i, "", char_ligne, _false);
                    if(files[1].i->lines[j].length < char_ligne)
                        print_space(char_ligne - files[1].i->lines[j].length);
                }

                print_space(char_center/2);
                if(p->left_column) {
                    fputs("(\n", stdout);
                } else {
                    fputc((int)' ', stdout);
                    print_space(char_center/2);

                    if(char_ligne > 0) {
                        print_lines_length(&files[1], j, j, "", char_ligne, _false);
                        if(files[1].i->lines[j].length < char_ligne)
                            print_space(char_ligne - files[1].i->lines[j].length);
                    }

                    fputc((int)'\n', stdout);
                }
            }

            i++;
            j++;
        }
    }
}

static void print_diff_ifdef(File files[]) {

    ulint i = 0, j = 0;
    ulint length_1 = 0, length_2 = 0;

    // For each line
    for(; i < files[0].i->line_max && j < files[1].i->line_max; i++, j++) {

        // One of them is changed
        if(files[0].i->lines[i].modified || files[1].i->lines[j].modified) {

            // Deleted
            if(files[0].i->lines[i].modified == LINE_DEL) {
                length_1 = diff_get_length(files[0].i, i);
                printf("#ifndef %s\n", p->ifdef);
                print_lines(&files[0], i, length_1 + i - 1, "", _true);
                printf("#endif /* %s */\n", p->ifdef);
            }
            // Added
            else if(files[1].i->lines[j].modified == LINE_ADD) {
                length_2 = diff_get_length(files[1].i, j);
                printf("#ifdef %s\n", p->ifdef);
                print_lines(&files[1], j, length_2 + j - 1, "", _true);
                printf("#endif /* %s */\n", p->ifdef);
            }
            // Changed
            else if(files[1].i->lines[j].modified == LINE_CHANGE) {
                length_1 = diff_get_length(files[0].i, i);
                length_2 = diff_get_length(files[1].i, j);

                printf("#ifndef %s\n", p->ifdef);
                print_lines(&files[0], i, length_1 + i - 1, "", _true);
                printf("#else /* %s */\n", p->ifdef);
                print_lines(&files[1], j, length_2 + j - 1, "", _true);
                printf("#endif /* %s */\n", p->ifdef);
            }
            // No change
            else {
                print_lines(&files[0], i, i, "", _true);
            }

            i += length_1;
            j += length_2;
        }
    }
}

void print_args(File files[]) {
    int i = 0;

    fputs("diff ", stdout);

    for(; i < 2; i++) {
        fputs(files[i].path, stdout);
        fputc(' ', stdout);
    }

    for(i = 1; i < p->argc; i++) {
        if(!(p->argv[i] == p->original_paths[0] || p->argv[i] == p->original_paths[1])) {
            fputs(p->argv[i], stdout);
            fputc(' ', stdout);
        }
    }

    fputc('\n', stdout);
}

void print_version(void) {
    printf("diff - version %s\nBy G. Villerez, Q. Ysambert, E. Berezovskiy and K. Maarek\n\nCompiled at %s on %s\n",VERSION_NUM, __TIME__,__DATE__);
    #ifdef DEBUG
        puts("(Compiled in debug version)");
    #endif
}

void print_help(void) {

    puts("Usage: diff [OPTION]... FILE1 FILE2\n\n"
         "  --normal                     output a normal diff (the default).\n"
         "  -c  -C NUM  --context[=NUM]  output NUM (default 3) lines of copied context.\n"
         "  -u  -U NUM  --unified[=NUM]  output NUM (default 3) lines of unified context.\n"
         "    -L LABEL --label LABEL         use Label instead of file name.\n"
         "    -p --show-c-function           show which C function each change is in.\n"
         "    -F RE --show-function-line=RE  show the most recent line matching RE.\n"
         "  -e  --ed                     output an ed script.\n"
         "  -n  --rcs                    output an RCS format diff.\n"
         "  -y  --side-by-side           output in two columns\n"
         "    -w NUM  --width=NUM          output at most NUM (default 130) characters per line.\n"
         "    --left-column                output only the left column of common lines.\n"
         "    --suppress-common-lines      do not output common lines.\n"
         "  --ifdef=NAME                 output merged file with '#ifdef NAME' diffs"
         "\n"
         "  -q  --brief                  Report only when files differ.\n"
         "  -s  --report-identical-file  Report when two files are the same.\n"
         "\n"
         "  -a  --text      treat all files as text.\n"
         "  --binary        treat all files as binary files.\n"
         "  -r --recursive  recursively compare any  subdirectories found.\n"
         "  -N --new-file   treat absent files as empty.\n"
         "\n"
         "  -i  --ignore-case                 consider upper- and lower-case to be the same.\n"
         "  -W  --ignore-all-space            ignore all white space.\n"
         "  -b  --ignore-space-change         ignore change in the amount of white space.\n"
         "  -B  --ignore-blank-lines          ignore change whose lines are all blank.\n"
         "  -I RE --ignore-matching-lines=RE  ignore change whose lines all match RE.\n"
         "\n"
        "  -t  --expand-tabs  expand tabs to spaces in output.\n"
        "  --tabsize=NUM      use NUM space (default 8) space for each tab.\n"
         "\n"
         "  --help        output this help.\n"
         "  -v --version  output version info."
         #ifdef DEBUG
         "\n\nDebug option :\n"
         "  --debug-show-options      show the struct containing program's arguments.\n"
         "  --debug-show-index        show the struct containing both file's index.\n"
         "  --debug-show-diff         show the raw diff result.\n"
         "  --debug-interactive-mode  pause the program after each step."
         #endif
         );

}
