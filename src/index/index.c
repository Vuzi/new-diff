#include "index.h"

/* == Prototypes == */
static ulint index_size(FILE *f, ulint* longest_line);
static void index_empty(File *file);
static END_LINE get_end_line(FILE *f, char c);


/* ===============================================
                     index_size

    Return the number of lines of the file f. Set
    longest_line to the the length of the longest
    line of the file.
    ----------------------------------------------
    FILE *f               : file
    u l int* longest_line : longest line
    ----------------------------------------------
    Return the number of lines in f.
   =============================================== */
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
    cpt++; // Last line

    rewind(f);

    return cpt;
}


/* ===============================================
                     get_end_line

    Return the type of end of line, and move the
    cursor to the end of the file.
    ----------------------------------------------
    FILE *f    : file
    char c     : char at the end of line
    ----------------------------------------------
    Return the end of line type.
   =============================================== */
static END_LINE get_end_line(FILE *f, char c) {

    if(c == '\n') {
        // UNIX line
        return LF;
    } else {
        c = getc(f);
        if(c == '\n') {
            // Windows line
            return CRLF;
        } else {
            // Mac OS 9 line

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


/* ===============================================
                     index_empty

    Create an index adapted to empty files.
    ----------------------------------------------
    FILE *f    : empy file
   =============================================== */
static void index_empty(File *file) {

    // Empty index creation
    file->i = (Index*)diff_xmalloc(sizeof(Index));

    file->i->line_max = 1;
    file->i->lines = (Line*)diff_xcalloc(sizeof(Line), 1);
    file->i->lines[0].end_line = END_OF_FILE;
    file->i->lines[0].h = HASH_START;
    file->i->lines[0].ignore = _false;
    file->i->lines[0].is_func = _false;
    file->i->lines[0].length = 0;
    file->i->lines[0].start = 0;
}


/* ===============================================
                     index_file

    Create an index to the file file.
    ----------------------------------------------
    FILE *f    : file to be indexed
   =============================================== */
void index_file(File *file) {

    ulint cpt = 0, j = 0, tab_cpt = 0, space_cpt = 0, blank_cpt = 0, max_lenght = 0;
    int tmp = 0, reg_stat = 0;
    hash_t h = HASH_START;

    char * buffer = NULL;

    if(file->empty) {
        index_empty(file);
        return;
    }

    // Index creation
    file->i = (Index*)diff_xmalloc(sizeof(Index));

    file->i->line_max = index_size(file->f, &max_lenght);
    file->i->lines = (Line*)diff_xcalloc(sizeof(Line), file->i->line_max);

    // Buffer
    buffer = (char*)diff_xmalloc(sizeof(char)*(max_lenght+1));

    if(file->f) {
        if(file->i->line_max > 0) {

            // First line start
            file->i->lines[j].start = 0;

            // Indexing
            while((tmp = getc(file->f)) != EOF) {
                // \n or \r
                if(IS_EL_START(tmp)) {
                    buffer[cpt - file->i->lines[j].start] = '\0';

                    // Test the function regex
                    if(p->show_regex_function) {
                        reg_stat = regexec(p->show_regex_function, buffer, (size_t) 0, NULL, 0);
                        if(!reg_stat)
                            file->i->lines[j].is_func = _true; // match
                        else if (reg_stat != REG_NOMATCH) {
                            send_error(NULL, "error while using the regex with line %d of file '%s'", j+1, file->path);
                            send_error(NULL, "function detection may not be working...");
                        }
                    }

                    // Ignoring blank lines
                    if(p->ignore_blank_lines) {
                        reg_stat = regexec(p->ignore_blank_lines, buffer, (size_t) 0, NULL, 0);
                        if(!reg_stat)
                            file->i->lines[j].ignore = _true; // match
                        else if (reg_stat != REG_NOMATCH) {
                            send_error(NULL, "error while using the regex with line %d of file '%s'", j+1, file->path);
                            send_error(NULL, "ignoring, blank line may not be working...");
                        }
                    }

                    // Ignoring lines matching regex
                    if(p->ignore_regex_match) {
                        reg_stat = regexec(p->ignore_regex_match, buffer, (size_t) 0, NULL, 0);
                        if(!reg_stat)
                            file->i->lines[j].ignore = _true; // match
                        else if (reg_stat != REG_NOMATCH) {
                            send_error(NULL, "error while using the regex with line %d of file '%s'", j+1, file->path);
                        }
                    }

                    // End of the line
                    file->i->lines[j].length = cpt - file->i->lines[j].start;
                    file->i->lines[j].end_line = get_end_line(file->f, tmp);

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

                    // New line start
                    if(file->i->lines[j++].end_line == CRLF) {
                        cpt++;
                        file->i->lines[j].start = ++cpt;
                    } else {
                        file->i->lines[j].start = ++cpt;
                    }

                }
                // Regular char
                else {

                    buffer[cpt - file->i->lines[j].start] = tmp;

                    // Case test
                    if(p->ignore_case) {
                        if(tmp >= 'A' && tmp <= 'Z')
                            tmp += 32;
                    }

                    // tab expansion
                    if(p->ignore_tab_change) {
                        if(tmp == '\t')
                            tab_cpt++;
                        else
                            tab_cpt = 0;
                    }

                    // Space change test
                    if(p->ignore_space_change) {
                        if(tmp == ' ')
                            space_cpt++;
                        else
                            space_cpt = 0;
                    }

                    // Blank space test
                    if(p->ignore_all_space) {
                        if(tmp == '\t' || tmp == ' ')
                            blank_cpt++;
                        else
                            blank_cpt = 0;
                    }

                    if(tmp < 0)
                        tmp = -tmp + 2;

                    if(blank_cpt <=1 && tab_cpt <= 1 && space_cpt <= 1)
                        h = hash(h, (char)tmp); // hashing

                    cpt++;
                }
            }

            // Last line ending
            file->i->lines[j].end_line = END_OF_FILE;
            file->i->lines[j].length = cpt - file->i->lines[j].start;
            file->i->lines[j].h = h;

            if(file->i->lines[j].length == 0) // If the file end with a new line char
                file->i->line_max--;
        }
    }

    free(buffer);
}


/* ===============================================
                     index_free

    Free the index of file.
    ----------------------------------------------
    FILE *f : file containing the index to be freed
   =============================================== */
void index_free(File *file) {
    if(file->i) {
        free(file->i->lines);
        free(file->i);
        file->i = NULL;
    }
}

#ifdef DEBUG
/* ===============================================
                     index_display

    Display the index of file.
    ----------------------------------------------
    FILE *f : file containing the index to be shown
   =============================================== */
void index_display(File *file) {

    ulint j = 0;

    puts("Index : ");

    if(file->i) {
        for(; j < file->i->line_max; j++) {
            printf("[%"SHOW_ulint"] start %05"SHOW_ulint" | length %05"SHOW_ulint" | hash %010"SHOW_ulint" | end_line %01d | is_func %01d | modified %01d | ignored %01d\n",
                   j, file->i->lines[j].start, file->i->lines[j].length, (ulint)file->i->lines[j].h,
                   file->i->lines[j].end_line, file->i->lines[j].is_func, file->i->lines[j].modified, file->i->lines[j].ignore);
        }
    }

}
#endif
