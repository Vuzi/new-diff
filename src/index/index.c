#include "index.h"

static ulint index_size(FILE *f, ulint* longest_line);
static void index_empty(File *file);
static END_LINE get_end_line(FILE *f, char c);

static ulint index_size(FILE *f, ulint* longest_line) {
    ulint cpt = 0, line_max = 0;
    int c = 0;

    *longest_line = 0;

    while((c = getc(f)) != EOF) {
        line_max++;
        if(IS_EL_START(c)) {
            get_end_line(f, c);
            cpt++;
            if(line_max > *longest_line)
                *longest_line = line_max;
            line_max = 0;
        }
    }
    cpt++; // Dernière ligne

    rewind(f);

    return cpt;
}

static END_LINE get_end_line(FILE *f, char c) {

    if(c == '\n') {
        /* Ligne UNIX */
        return LF;
    } else {
        c = getc(f);
        if(c == '\n') {
            /* Ligne Windows */
            return CRLF;
        } else {
            /* Ligne Mac OS 9 */

            #ifdef _WIN32
            fseeko64(f, (long long)-1, SEEK_CUR);
            #else
            fseeko(f, (long long)-1, SEEK_CUR);
            #endif
            return CR;
        }
    }

    return -1;
}

static void index_empty(File *file) {

    /* Création de l'index */
    file->i = (Index*)malloc(sizeof(Index));

    file->i->line_max = 1;
    file->i->lines = (Line*)calloc(sizeof(Line), 1);
    file->i->lines[0].end_line = END_OF_FILE;
    file->i->lines[0].h = HASH_START;
    file->i->lines[0].ignore = _false;
    file->i->lines[0].is_func = _false;
    file->i->lines[0].length = 0;
    file->i->lines[0].start = 0;
}

void index_file(File *file) {

    ulint cpt = 0, j = 0, tab_cpt = 0, space_cpt = 0, blank_cpt = 0, max_lenght = 0;
    int tmp = 0, reg_stat = 0;
    hash_t h = HASH_START;

    char * buffer = NULL;

    if(file->empty) {
        index_empty(file);
        return;
    }

    /* Création de l'index */
    file->i = (Index*)malloc(sizeof(Index));

    file->i->line_max = index_size(file->f, &max_lenght);
    file->i->lines = (Line*)calloc(sizeof(Line), file->i->line_max);

    /* Buffer temporaire */
    buffer = (char*)malloc(sizeof(char)*(max_lenght+1));

    if(file->f) {
        if(file->i->line_max > 0) {

            /* Début première ligne */
            file->i->lines[j].start = 0;

            /* Indexation */
            while((tmp = getc(file->f)) != EOF) {
                /* \n ou \r */
                if(IS_EL_START(tmp)) {
                    /* Ajout au buffer */
                    buffer[cpt - file->i->lines[j].start] = '\0';

                    /* Test regex fonction */
                    if(p->show_regex_function) {
                        reg_stat = regexec(p->show_regex_function, buffer, (size_t) 0, NULL, 0);
                        if(!reg_stat)
                            file->i->lines[j].is_func = _true; // match
                        else if (reg_stat != REG_NOMATCH) {
                            send_error(NULL, "error while using the regex with line %d of file '%s'", j+1, file->path);
                            send_error(NULL, "function detection may not be working...");
                        }
                    }

                    /* On ignore les lignes blanches */
                    if(p->ignore_blank_lines) {
                        reg_stat = regexec(p->ignore_blank_lines, buffer, (size_t) 0, NULL, 0);
                        if(!reg_stat)
                            file->i->lines[j].ignore = _true; // match
                        else if (reg_stat != REG_NOMATCH) {
                            send_error(NULL, "error while using the regex with line %d of file '%s'", j+1, file->path);
                            send_error(NULL, "ignoring blank line may not be working...");
                        }
                    }

                    /* On ignore les lignes qui match le regex */
                    if(p->ignore_regex_match) {
                        reg_stat = regexec(p->ignore_regex_match, buffer, (size_t) 0, NULL, 0);
                        if(!reg_stat)
                            file->i->lines[j].ignore = _true; // match
                        else if (reg_stat != REG_NOMATCH) {
                            send_error(NULL, "error while using the regex with line %d of file '%s'", j+1, file->path);
                        }
                    }

                    /* Fin de l'ancienne ligne */
                    file->i->lines[j].length = cpt - file->i->lines[j].start;
                    file->i->lines[j].end_line = get_end_line(file->f, tmp);

                    /* Hashage du/des caracs de fin de ligne */
                    if(p->strip_trailing_cr || file->i->lines[j].end_line == LF) {
                        h = hash(h, (char)'\n');
                    } else if (file->i->lines[j].end_line == CR) {
                        h = hash(h, (char)'\r');
                    } else {
                        h = hash(h, (char)'\r');
                        h = hash(h, (char)'\n');
                    }

                    file->i->lines[j].h = h;
                    h = HASH_START;

                    tab_cpt = 0;
                    space_cpt = 0;
                    blank_cpt = 0;

                    /* Début nouvelle ligne */
                    if(file->i->lines[j++].end_line == CRLF) {
                        cpt++;
                        file->i->lines[j].start = ++cpt;
                    } else {
                        file->i->lines[j].start = ++cpt;
                    }

                }
                /* Caractère normal */
                else {

                    /* Ajout au buffer */
                    buffer[cpt - file->i->lines[j].start] = tmp;

                    /* Test de casse */
                    if(p->ignore_case) {
                        if(tmp >= 'A' && tmp <= 'Z')
                            tmp += 32;
                    }

                    /* Test expansion tabulation */
                    if(p->ignore_tab_change) {
                        if(tmp == '\t')
                            tab_cpt++;
                        else
                            tab_cpt = 0;
                    }

                    /* Test changement espaces */
                    if(p->ignore_space_change) {
                        if(tmp == ' ')
                            space_cpt++;
                        else
                            space_cpt = 0;
                    }

                    /* Test d'ignorance des espaces blancs */
                    if(p->ignore_all_space) {
                        if(tmp == '\t' || tmp == ' ')
                            blank_cpt++;
                        else
                            blank_cpt = 0;
                    }

                    if(blank_cpt <=1 && tab_cpt <= 1 && space_cpt <= 1)
                        h = hash(h, (char)tmp); // hashage

                    cpt++;
                }
            }

            /* Fin dernière ligne */
            file->i->lines[j].end_line = END_OF_FILE;
            file->i->lines[j].length = cpt - file->i->lines[j].start;
            file->i->lines[j].h = h;

            if(file->i->lines[j].length == 0) // Si fin de fichier avec saut de ligne
                file->i->line_max--;
        }
    }

    free(buffer);
}

void index_free(File *file) {
    if(file->i) {
        free(file->i->lines);
        free(file->i);
        file->i = NULL;
    }
}

#ifdef DEBUG
void index_display(File *file) {

    ulint j = 0;

    puts("Index : ");

    if(file->i) {
        for(; j < file->i->line_max; j++) {
            printf("[%"SHOW_ulint"] start %05"SHOW_ulint" | length %05"SHOW_ulint" | hash %010"SHOW_ulint" | end_line %01d | is_func %01d | modified %01d\n",
                   j, file->i->lines[j].start, file->i->lines[j].length, (ulint)file->i->lines[j].h,
                   file->i->lines[j].end_line, file->i->lines[j].is_func, file->i->lines[j].modified);
        }
    }

}
#endif
