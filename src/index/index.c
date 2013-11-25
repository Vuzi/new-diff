#include "index.h"

static ulint index_size(FILE *f);
static END_LINE get_end_line(FILE *f, char c);

static ulint index_size(FILE *f) {
    ulint cpt = 0;
    int c = 0;

    while((c = getc(f)) != EOF) {
        if(IS_EL_START(c)) {
            get_end_line(f, c);
            cpt++;
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
            fseeko64(f, (long long)-1, SEEK_CUR);
            return CR;
        }
    }

    return -1;
}

void index_file(File *file) {

    ulint cpt = 0, j = 0;
    _bool new_line = _true;
    int tmp = 0;
    hash_t h = HASH_START;

    /* Création de l'index */
    file->i = (Index*)malloc(sizeof(Index));

    file->i->line_max = index_size(file->f);
    file->i->lines = (Line*)calloc(sizeof(Line), file->i->line_max);

    if(file->f) {
        if(file->i->line_max > 0) {

            /* Début première ligne */
            file->i->lines[j].start = 0;

            /* Indexation */
            while((tmp = getc(file->f)) != EOF) {
                /* \n ou \r */
                if(IS_EL_START(tmp)) {
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

                    /* Début nouvelle ligne */
                    new_line = _true;
                    if(file->i->lines[j++].end_line == CRLF) {
                        cpt++;
                        file->i->lines[j].start = ++cpt;
                    } else {
                        file->i->lines[j].start = ++cpt;
                    }

                }
                /* Caractère normal */
                else {

                    /* Fonctions C */
                    if(new_line) {
                        new_line = _false;
                        if((tmp >= 'a' && tmp <= 'z') ||
                           (tmp >= 'A' && tmp <= 'Z') ||
                           (tmp == '_')){
                            file->i->lines[j].is_c_func = _true;
                        }
                    }

                    /* Ici : gérer les options de blank space */
                    cpt++;
                    h = hash(h, (char)tmp); // hashage
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

    puts("Affichage Index : ");

    if(file->i) {
        for(; j < file->i->line_max; j++) {
            printf("[%"SHOW_ulint"] start %05"SHOW_ulint" | length %05"SHOW_ulint" | hash %010"SHOW_ulint" | end_line %01d | is_c_func %01d | modified %01d\n",
                   j, file->i->lines[j].start, file->i->lines[j].length, (ulint)file->i->lines[j].h,
                   file->i->lines[j].end_line, file->i->lines[j].is_c_func, file->i->lines[j].modified);
        }
    }

}
#endif
