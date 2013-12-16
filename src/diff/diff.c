#include "diff.h"

/* == Statics prototypes == */
static _bool is_binary(FILE *f);

static _bool diff_file_binary(FILE *f1, FILE* f2);
static _bool diff_file_brief(Index *i1, Index *i2);

static void diff_analyse_index(File files[]);
static void diff_file_LCS_matrix(Line **LCS_lines[], ulint len[]);
static void diff_file_LCS_Myers(Line **LCS_lines[], ulint len[]);
static _bool diff_file_regular(File files[]);

static const char* get_type_char(struct stat* s);
static lint dir_search(const char* to_search, DIR* d);
static int diff_dir_make(File files[]);
static int diff_dir_make_r(File files[], struct dirent *dr[]);
static int diff_dir_make_r_new_file(File files[], struct dirent *dr[]);

/* ===============================================
                      sec_fopen

    Secure open of path using mod mod. If the
    fopen isn't successful, the error is displayed.
    ----------------------------------------------
    c char *path : path
    c char *mode : open mod
    ----------------------------------------------
    Return the result of fopen.
   =============================================== */
FILE* sec_fopen(const char* path, const char* mod) {
    FILE* f = fopen(path, mod);

    if(!f)
        send_error(path, NULL, NULL);

    return f;
}


/* ===============================================
                      sec_fclose

    Secure close of f.
    ----------------------------------------------
    FILE *f : Flux to close
   =============================================== */
void sec_fclose(FILE *f) {

    if(f) {
        if(fclose(f) != 0)
            send_error("fclose()", NULL, NULL);
    }
}


/* ===============================================
                      sec_closedir

    Secure close of d.
    ----------------------------------------------
    DIR *f : Directory to close
   =============================================== */
void sec_closedir(DIR *d) {

    if(d) {
        if(closedir(d) == -1)
            send_error("closedir()", NULL, NULL);
    }
}


/* ===============================================
                      sec_opendir

    Secure open of directory. If the diropen isn't
    successful, the error is displayed.
    ----------------------------------------------
    c char *path : path
    ----------------------------------------------
    Return the result of opendir.
   =============================================== */
DIR *sec_opendir(const char* path) {

    DIR *d = opendir(path);

    if(!d)
        send_error(path, NULL, NULL);

    return d;
}


/* ===============================================
                      free_diff

    Free the elements in the diff at the end of
    the program.
    ----------------------------------------------
    files : array of the two files to be freed
   =============================================== */
void free_diff(File files[]) {

    sint i = 0;

    for(; i < 2; i++) {
        if(files[i].i)
            index_free(&files[i]);

        if(files[i].label == files[i].path)
            free(files[i].label);
        else
            free(files[i].label), free(files[i].path);

        files[i].label = files[i].path = NULL;
    }
}


/* ===============================================
                      free_diff_r

    Free the elements in the diff at the end of
    the program. Only used recursivly.
    ----------------------------------------------
    files : array of the two files to be freed
   =============================================== */
void free_diff_r(File files[]) {

    sint i = 0;

    for(; i < 2; i++) {
        if(files[i].i)
            index_free(&files[i]);
    }
}


/* ===============================================
                    diff_file_brief

    Compare quicly two index to find if they are
    identicals. Stop avec the first difference.
    ----------------------------------------------
    Index i1 : index n°1
    Index i2 : index n°2
    ----------------------------------------------
    Return _true if the files are different,
    otherwise _false.
   =============================================== */
static _bool diff_file_brief(Index *i1, Index *i2) {
    ulint j = 0;

    // If we have the same amount of lines
    if(i1->line_max == i2->line_max) {
        // Compare every lines
        for(; j < i1->line_max; j++) {
            if(i1->lines[j].h != i2->lines[j].h)
                return _true;
        }
        return _false;
    } else
        return _true;
}


/* ===============================================
                    is_binary

    Try to guess if the file is binary by reading
    the first 4 bytes. Thoses bytes should be at
    0 for a binary file.
    ----------------------------------------------
    FILE *f : file to test
    ----------------------------------------------
    Return _true if the file is detected as binary,
    _false otherwise.
   =============================================== */
static _bool is_binary(FILE *f) {

    int i = 0, c = 0;

    if(f) {
        c = getc(f);

        while(i < 4 && c == 0 && c != EOF) {
            i++;
            c = getc(f);
        }

        rewind(f);

        return (i < 4 ? _false : _true);
    } else
        return _false; // If empty, not binary
}


/* ===============================================
                 diff_file_binary

    diff of two binary file, which compare every
    bytes of the two files.
    ----------------------------------------------
    FILE *f1 : file n°1
    FILE *f2 : file n°2
    ----------------------------------------------
    Return _true if the files are different,
    otherwise _false.
   =============================================== */
static _bool diff_file_binary(FILE *f1, FILE* f2) {

    int c1 = 0, c2 = 0;

    // If one of them (or both) is empty
    if(!f1 || !f2) {
        if(f1 == f2)
            return _false;
        else
            return _true;
    }

    fseek(f1, 0, SEEK_END);
    fseek(f2, 0, SEEK_END);

    // If they have the same size
    if(ftell(f1) == ftell(f2)) {

        rewind(f1);
        rewind(f2);

        // Compare every bytes
        while((c1 = getc(f1)) == (c2 = getc(f2))) {
            if(c1 == EOF)
                return _false;
        }
    }

    return _true;
}


/* ===============================================
                 diff_get_length

    Count the number of different lines from the
    line i.
    ----------------------------------------------
    Index *index : index to analyse
    ulint i      : start of the diff zone
    ----------------------------------------------
    Return the last modified index.
   =============================================== */
ulint diff_get_length(Index* index, ulint i) {

    ulint k = i;

    // We search the lenght of the change
    for(; k < index->line_max && index->lines[k].modified; k++);

    return k - i;
}


/* ===============================================
                 diff_analyse_index

    Analyse a file, and detect added, deleted and
    modified lines. This operation is done in order
    to make easier the display of the file.
    ----------------------------------------------
    Index *i1 : index n°1
    Index *i2 : index n°2
   =============================================== */
static void diff_analyse_index(File files[]) {

    ulint i = 0, j = 0, k = 0;

    ulint length_1 = 0, length_2 = 0;

    // For every line
    for(; i < files[0].i->line_max && j < files[1].i->line_max; i++, j++) {

        // If one of them is changed
        if(files[0].i->lines[i].modified || files[1].i->lines[j].modified) {

            length_1 = diff_get_length(files[0].i, i);
            length_2 = diff_get_length(files[1].i, j);

            // Deletions
            if(length_2 == 0 || files[1].empty) {
                for(k = i; k < i+length_1; k++)
                    files[0].i->lines[k].modified = LINE_DEL;
            }
            // Additions
            else if (length_1 == 0 || files[0].empty) {
                for(k = j; k < j+length_2; k++)
                    files[1].i->lines[k].modified = LINE_ADD;
            }
            // Changes
            else {
                for(k = i; k < i+length_1; k++)
                    files[0].i->lines[k].modified = LINE_CHANGE;
                for(k = j; k < j+length_2; k++)
                    files[1].i->lines[k].modified = LINE_CHANGE;
            }

            // And we go back at the end of the change
            i += length_1;
            j += length_2;
        }
    }
}


/* ===============================================
                 diff_file_LCS_Myers

    Diff using the Myers' LCS algorithm. Rather
    than using matrix to store data, this one
    only store 'snakes'. The algorithm stop after
    the first snake that reach the end (which
    is (one of) the solution(s)).
    This algorithm is faster than the matrix in
    usual diff case, and take usually a lot less
    space in memory.
    The function have no return value, and set
    the lines modified value to 1 when they are
    modified, otherwise its stays to 0.
    ----------------------------------------------
    Line **LCS_lines[] : array of array of lines*
    u l int len[]      : size of the array of lines*
   =============================================== */
static void diff_file_LCS_Myers(Line **LCS_lines[], ulint len[]) {

    // Snake list
    Snake *snakes = NULL, *first_snake = NULL;

    // Last values
    Snake **_V = diff_xmalloc(sizeof(Snake *)*(1 + 2*(len[0] + len[1])));
    Snake **V = _V + len[0] + len[1]; // So we can access using -d to d to access data

    // Temps values
    lint k = 0, d = 0, i = 0, j = 0;

    snake_add(&snakes, 0, -1, 0, 0, 0, 0); // Stub for d = 0
    first_snake = V[1] = snakes;

    for(d = 0; d <= (signed)len[0] + (signed)len[1]; d++) {
        for(k = -d; k <= d; k += 2) {
            // Down or right ?
            char down = (k == -d || (k != d && V[k-1]->end.x < V[k+1]->end.x));

            int kPrev = down ? k + 1: k - 1;

            // Start point
            int xStart = V[kPrev]->end.x;
            int yStart = xStart - kPrev;

            // Mid point
            int xMid = down ? xStart : xStart + 1;
            int yMid = xMid - k;

            // End point
            int xEnd = xMid;
            int yEnd = yMid;

            // Follow diagonal
            while( xEnd < (signed)len[0] && yEnd < (signed)len[1] &&
                  LCS_lines[0][xEnd]->h == LCS_lines[1][yEnd]->h &&
                  LCS_lines[0][xEnd]->length == LCS_lines[1][yEnd]->length )
                xEnd++, yEnd++;

            // Save the snake
            snake_add(&snakes, xStart, yStart, xMid, yMid, xEnd, yEnd);
            snakes->path = V[kPrev];
            V[k] = snakes;

            // Check for solutiontu c
            if(xEnd >= (signed)len[0] && yEnd >= (signed)len[1]) // Solution has been found
                goto solution_found;
        }
    }

    // Here need to check data
    solution_found:

    // Set every line to changed, because we'll find the unchanged lines
    for( i = 0; i < 2; i++) {
        for( j = 0; j < (signed)len[i]; j++)
            LCS_lines[i][j]->modified = LINE_IS_CHANGED;
    }

    // Now we have our solution at V[k], we can analyse it
    snakes = V[k];

    // We'll analyse each snake until we get back to the first snake. Also check for NULL snake
    while(snakes && (snakes->end.x > 0 || snakes->end.y > 0)) {
        //Firstly we analyse the diagonal of the snake (Which is part of the LCS)
        for(i = snakes->end.x, j = snakes->end.y; i > snakes->mid.x; i--, j--) {
            LCS_lines[0][i-1]->modified = LINE_NO_CHANGE;
            LCS_lines[1][j-1]->modified = LINE_NO_CHANGE;
        }

        // Then we find the previous linked snake
        snakes = snakes->path;
    }

    // Fre values
    free(_V);
    snake_clear(first_snake);
}


/* ===============================================
                 diff_file_LCS_matrix

    Diff using usual matricial LCS. This algorithm
    is slow in every case, and can take a lot ( too
    much) of memory to allocate its matrix.
    But this algorithm is strong and well tested.
    It can be used with the option --use-matrix-lcs.
    ----------------------------------------------
    Line **LCS_lines[] : array of array of lines*
    u l int len[]      : size of the array of lines*
   =============================================== */
static void diff_file_LCS_matrix(Line **LCS_lines[], ulint len[]) {

    // We initialize the matrix
    ulint i = 0, j = 0;

    ulint *matrix_tmp = (ulint*)diff_xcalloc(sizeof(ulint), (len[0]+1) * (len[1]+1));
    ulint **matrix = (ulint**)diff_xcalloc(sizeof(ulint*), len[0]+1);

    for(; i <= len[0]; i++)
        matrix[i] = ( i * (len[1]+1)) + matrix_tmp;

    // Fill the matrix
    for(i = 0; i < len[0]; i++) {
        for(j = 0; j < len[1]; j++) {
            if(LCS_lines[0][i-1]->h == LCS_lines[1][j-1]->h && LCS_lines[0][i-1]->length == LCS_lines[1][j-1]->length)
                matrix[i][j] = matrix[i-1][j-1] + 1;
            else
                matrix[i][j] = matrix[i-1][j] > matrix[i][j-1] ? matrix[i-1][j] : matrix[i][j-1];
        }
    }

    // Set every line to changed, because we'll find the unchanged lines
    for( i = 0; i < 2; i++) {
        for( j = 0; j < len[i]; j++)
            LCS_lines[i][j]->modified = LINE_IS_CHANGED;
    }

    // Compute the LCS
    for(i = len[0], j = len[1]; matrix[i][j] != 0 && i > 0 && j > 0;) {
        // If we can go to left
        if(matrix[i-1][j] == matrix[i][j])
            i--;
        // If we can go up
        else if(matrix[i][j-1] == matrix[i][j])
            j--;
        // We need to take the bridge
        else {
            // Valeur dans le LCS
            LCS_lines[0][i-1]->modified = LCS_lines[1][j-1]->modified = LINE_NO_CHANGE;
            i--, j--;
        }
    }

    // Free of the matrix
    free(matrix[0]);
    free(matrix);
}

/* ===============================================
                  diff_file_regular

    Diff between two regular indexed files.
    ----------------------------------------------
    File files[] : files to be computed.
    ----------------------------------------------
    Return _true when files are identical, _false
    otherwise.
   =============================================== */
static _bool diff_file_regular(File files[]) {

    ulint i = 0, j = 0, k = 0;

    ulint len[2] = {0, 0};
    Line **LCS_lines[2] = { NULL, NULL };

    // Ignore first identicals lines
    for( i = 0; i < files[0].i->line_max && i < files[1].i->line_max; i++) {
        if(files[0].i->lines[i].h == files[1].i->lines[i].h) {
            files[0].i->lines[i].ignore = files[1].i->lines[i].ignore = _true;
        } else
            break;
    }

    // Ignore last identicals lines
    for( i = files[0].i->line_max-1, j = files[1].i->line_max-1; i > 0 && j > 0; i--, j--) {
        if(files[0].i->lines[i].h == files[1].i->lines[j].h && !files[0].i->lines[i].ignore) { // If line ignored, stop
            files[0].i->lines[i].ignore = files[1].i->lines[j].ignore = _true;
        } else
            break;
    }

    // Count the new number of lines
    for(i = 0; i < 2; i++) {
        for( j = 0; j < files[i].i->line_max; j++) {
            if(!files[i].i->lines[j].ignore) // Count if we don't ignore it
                len[i]++;
        }
    }

    // If we have something to test
    if(len[0] > 0 || len[1] > 0) {
        // Alloc the new tab
        for(i = 0; i < 2; i++) {
            LCS_lines[i] = diff_xmalloc(sizeof(Line*)*len[i]);
            // Add to the new tab the lines not ignored
            for(j = 0, k = 0; j < files[i].i->line_max; j++) {
                if(!files[i].i->lines[j].ignore) {
                    LCS_lines[i][k] = &files[i].i->lines[j];
                    k++;
                }
            }
        }

        // Do the LCS stuff
        if(p->use_matrix_lcs)
            diff_file_LCS_matrix(LCS_lines, len);
        else
            diff_file_LCS_Myers(LCS_lines, len);

        // Free what need to be freed
        free(LCS_lines[0]);
        free(LCS_lines[1]);
    }

    if(files[0].i->line_max == files[1].i->line_max && len[0] == 0 && len[1] == 0) // Same file
        return _false;
    else {
        // Analyse the index to detect additions, deletions and changes
        diff_analyse_index(files);
        return _true;
    }
}


/* ===============================================
                 get_type_char

    Allow us to easily get the type of a file
    using stat.
    ----------------------------------------------
    stat *s : stat to test
    ----------------------------------------------
    Return the name of the type in a static string.
   =============================================== */
static const char* get_type_char(struct stat *s) {

    if(S_ISREG(s->st_mode)) {
        if(s->st_size == 0)
            return "regular empty file";
        else
            return "regular file";
    } else if (S_ISDIR(s->st_mode))
        return "directory";
    else if (S_ISCHR(s->st_mode))
        return "character device";
    else if (S_ISBLK(s->st_mode))
        return "block device";
    else if (S_ISFIFO(s->st_mode))
        return "FIFO (named pipe)";
    #ifdef S_ISLNK
    else if (S_ISLNK(s->st_mode))
        return "symbolic link";
    #endif
    #ifdef S_ISSOCK
    else if (S_ISSOCK(s->st_mode))
        return "socket";
    #endif
    else
        return "unknown/other";
}


/* ===============================================
                    dir_search

    Locate a file in a directory DIR.
    ----------------------------------------------
    const char* to_search : name to search
    DIR* d                : where to search
    ----------------------------------------------
    Return the index of to_search in d, otherwise
    -1.
   =============================================== */
static lint dir_search(const char* to_search, DIR* d) {

    lint d_save = telldir(d), d_val = 0;
    struct dirent *dr = NULL;

    while ((dr = readdir(d)) != NULL) {
        if(diff_strcmp(to_search, dr->d_name) == 0) {
            d_val = telldir(d);
            seekdir(d, d_save);
            return d_val;
        }
    }

    seekdir(d, d_save);
    return -1;
}


/* ===============================================
             diff_dir_make_r_new_file

    Analyse recursivly two files, whith one of them
    being null.
    ----------------------------------------------
    Files files[]       : files
    struct dirent *dr[] : files to test
    ----------------------------------------------
    Return the result of the recursiv call.
   =============================================== */
static int diff_dir_make_r_new_file(File files[], struct dirent *dr[]) {

    File new_files[2];
    char *tmp_names[2] = {NULL, NULL};
    char *labels[2] = {NULL, NULL};
    struct stat st[2], *st_notempty = NULL;
    int ret = EXIT_IDENTICAL_FILES, i = 0;

    // We stat the existing file
    for(i = 0; i < 2; i++) {
        if (dr[i]) {
            // Name
            tmp_names[i] = diff_xmalloc(sizeof(char)*(diff_strlen(dr[i]->d_name)+diff_strlen(files[i].path)+2));
            sprintf(tmp_names[i], "%s/%s", files[i].path, dr[i]->d_name);
            // We stat the file
            if(stat(tmp_names[i], &(st[i])) != 0) {
                send_error(tmp_names[i], NULL);
                ret = EXIT_DIFFERENTS_FILES;
            }
            st_notempty = &st[i];
        }
    }

    if(ret == EXIT_IDENTICAL_FILES && st_notempty) {
        if(!(S_ISREG(st_notempty->st_mode) ||
           (p->recursive_dir && S_ISDIR(st_notempty->st_mode))))
            ret = EXIT_IDENTICAL_FILES;
        else {

            #ifdef DEBUG
                printf("Start of sub-file comparison\n");
            #endif

            // We create the new names and labels to test
            if(!dr[0]) {
                tmp_names[0] = (char*)diff_xmalloc(sizeof(char)*(diff_strlen(dr[1]->d_name)+diff_strlen(files[0].path)+2));
                sprintf(tmp_names[0], "%s/%s", files[0].path, dr[1]->d_name);
            } else {
                tmp_names[1] = (char*)diff_xmalloc(sizeof(char)*(diff_strlen(dr[0]->d_name)+diff_strlen(files[1].path)+2));
                sprintf(tmp_names[1], "%s/%s", files[1].path, dr[0]->d_name);
            }

            labels[0] = ( files[0].path != files[0].label ? files[0].label : NULL );
            labels[1] = ( files[1].path != files[1].label ? files[1].label : NULL );

            p->in_recur++;
            init_diff_r(tmp_names, labels, new_files);
            ret = (S_ISDIR(st_notempty->st_mode) ? diff_dir(new_files) : diff_file(new_files));
            free_diff_r(new_files);
            p->in_recur--;

            #ifdef DEBUG
                printf("End of comparison\n--------------\n");
            #endif
        }
    }

    if(tmp_names[0]) free(tmp_names[0]);
    if(tmp_names[1]) free(tmp_names[1]);

    return ret;
}


/* ===============================================
                   diff_dir_make_r

    Analyse recursivly two files.
    ----------------------------------------------
    Files files[]       : files
    struct dirent *dr[] : files to test
    ----------------------------------------------
    Return the result of the recursiv call.
   =============================================== */
static int diff_dir_make_r(File files[], struct  dirent *dr[]) {

    File new_files[2];
    char *tmp_names[2] = {NULL, NULL};
    char *labels[2] = {NULL, NULL};
    struct stat st[2];
    int ret = EXIT_IDENTICAL_FILES, i = 0;

    // We stat both files
    for(i = 0; i < 2; i++) {
        // Names
        tmp_names[i] = diff_xmalloc(sizeof(char)*(diff_strlen(dr[i]->d_name)+diff_strlen(files[i].path)+3));
        sprintf(tmp_names[i], "%s/%s", files[i].path, dr[i]->d_name);

        // We stat the file
        if(stat(tmp_names[i], &(st[i])) != 0) {
            send_error(tmp_names[i], NULL);
            ret = EXIT_DIFFERENTS_FILES;
        }
    }

    if(ret == EXIT_IDENTICAL_FILES) {
        // Same types
        if(st[0].st_mode == st[1].st_mode) {
            // Types we can't compare
            if(!S_ISREG(st[0].st_mode) && !(p->recursive_dir && S_ISDIR(st[0].st_mode))) {
                ret = EXIT_IDENTICAL_FILES;
            } else {
                #ifdef DEBUG
                    printf("Start of sub-file comparison\n");
                #endif

                // We create the new labels to test
                for(i = 0; i < 2; i++)
                    labels[i] = ( files[i].path != files[i].label ? files[i].label : NULL );

                p->in_recur++;
                init_diff_r(tmp_names, labels, new_files);
                ret = (S_ISDIR(st[0].st_mode) ? diff_dir(new_files) : diff_file(new_files));
                free_diff_r(new_files);
                p->in_recur--;

                #ifdef DEBUG
                    printf("End of comparison\n--------------\n");
                #endif
            }
        }
        // Not the same types
        else {
            printf("File %s/%s is a %s while file %s/%s is a %s\n",
                    files[0].label, dr[0]->d_name, get_type_char(&st[0]),
                    files[1].label, dr[1]->d_name, get_type_char(&st[1]));
            ret = EXIT_DIFFERENTS_FILES;
        }
    }

    free(tmp_names[0]);
    free(tmp_names[1]);

    return ret;
}


/* ===============================================
                   diff_dir_make

    Analyse recursivly two directory.
    ----------------------------------------------
    Files files[]       : files to test
    ----------------------------------------------
    Return if the directory are identical or not
   =============================================== */
static int diff_dir_make(File files[]) {

    int ret = EXIT_IDENTICAL_FILES, tmp = 0, i = 0;
    struct dirent *dr[2] = {NULL, NULL};

    #ifdef DEBUG
        printf("Start of directories comparison...\n");

        if(!p->recursive_dir)
            START_TIMER;
    #endif

    // We skip . & ..
    for(; i < 2; i++) {
        if(!files[i].empty)
            seekdir(files[i].d, 2);
    }

    // For every file in d1
    while (!files[0].empty && (dr[0] = readdir(files[0].d)) != NULL) {
        if(!files[1].empty && (dr[1] = readdir(files[1].d)) != NULL) {
            // Same name
            if((!p->ignore_case_filename && diff_strcmp(dr[0]->d_name, dr[1]->d_name) == 0) ||
                diff_strcasecmp(dr[0]->d_name, dr[1]->d_name) == 0)
            {
                ret = ((tmp = diff_dir_make_r(files, dr)) > ret ? tmp : ret);
            }
            // Different name
            else {
                // d1 is further in d2
                if((tmp = dir_search(dr[0]->d_name, files[1].d)) != -1) {
                    // Every file in d2 until tmp aren't in d1
                    dr[0] = NULL; // No file to compare to
                    do {
                        if(p->new_file)
                            ret = ((tmp = diff_dir_make_r_new_file(files, dr)) > ret ? tmp : ret );
                        else
                            ret = EXIT_DIFFERENTS_FILES, printf("Only in %s: %s\n", files[1].path, dr[1]->d_name);
                    } while((dr[1] = readdir(files[1].d)) != NULL && telldir(files[1].d) < tmp);
                    seekdir(files[0].d, telldir(files[0].d)-1);
                    seekdir(files[1].d, telldir(files[1].d)-1);
                }
                // d2 is further in d1
                else if((tmp = dir_search(dr[1]->d_name, files[0].d)) != -1) {
                    // Every file in d1 until tmp arent in d2
                    dr[1] = NULL; // No file to compare to
                    do {
                        if(p->new_file)
                            ret = ((tmp = diff_dir_make_r_new_file(files, dr)) > ret ? tmp : ret );
                        else
                            ret = EXIT_DIFFERENTS_FILES, printf("Only in %s: %s\n", files[0].path, dr[0]->d_name);
                    } while((dr[0] = readdir(files[0].d)) != NULL && telldir(files[0].d) < tmp);
                    seekdir(files[0].d, telldir(files[0].d)-1);
                    seekdir(files[1].d, telldir(files[1].d)-1);
                }
                // d2 is not further in d1 and vice-versa
                else {
                    printf("Only in %s: %s\n", files[0].path, dr[0]->d_name);
                    printf("Only in %s: %s\n", files[1].path, dr[1]->d_name);
                    ret = EXIT_DIFFERENTS_FILES;
                }
            }
        } else {
            // Only in d1
            if(p->new_file)
                ret = ((tmp = diff_dir_make_r_new_file(files, dr)) > ret ? tmp : ret );
            else
                ret = EXIT_DIFFERENTS_FILES, printf("Only in %s: %s\n", files[0].path, dr[0]->d_name);
        }
    }

    // Only in d2
    while((dr[1] = readdir(files[1].d)) != NULL) {
        if(p->new_file)
            ret = ((tmp = diff_dir_make_r_new_file(files, dr)) > ret ? tmp : ret );
        else
            ret = EXIT_DIFFERENTS_FILES, printf("Only in %s: %s\n", files[0].path, dr[0]->d_name);
    }

    #ifdef DEBUG
        if(!p->recursive_dir) {
            STOP_TIMER;
            if(ret == EXIT_DIFFERENTS_FILES)
                printf("Directories are different\n...comparison of directories completed (%.4fs) \n--------------\n", GET_TIMER_VALUE);
            else
                printf("Directories are identical\n...comparison of directories completed (%.4fs) \n--------------\n", GET_TIMER_VALUE);
        } else {
            if(ret == EXIT_DIFFERENTS_FILES)
                printf("Directories are different\n...comparison of directories completed\n--------------\n");
            else
                printf("Directories are identical\n...comparison of directories completed\n--------------\n");
        }
    #endif

    return ret;
}


/* ===============================================
                    diff_dir

    Core function of the diff between two dirs.
    ----------------------------------------------
    Files files[]       : files to test
    ----------------------------------------------
    Return if the directory are identical or not
   =============================================== */
int diff_dir(File files[]) {

    int ret = EXIT_IDENTICAL_FILES;

    // Works because we already check the two paths
    if(files[0].empty || (files[0].d = sec_opendir(files[0].path))) {
        if(files[1].empty || (files[1].d = sec_opendir(files[1].path))) {
            ret = diff_dir_make(files);
            if(files[0].d) sec_closedir(files[0].d);
            if(files[1].d) sec_closedir(files[1].d);
        } else {
            if(files[0].d) sec_closedir(files[0].d);
            ret = EXIT_ERROR;
        }
    } else
        ret = EXIT_ERROR;

    return ret;
}



/* ===============================================
                     diff_file

    Core function of the diff between two files.
    ----------------------------------------------
    Files files[]       : files to test
    ----------------------------------------------
    Return if the files are identical or not
   =============================================== */
int diff_file(File files[]) {

    int ret = EXIT_IDENTICAL_FILES;

    if(files[0].empty || (files[0].f = sec_fopen(files[0].path, "rb+"))) {
        if(files[1].empty || (files[1].f = sec_fopen(files[1].path, "rb+"))) {

            // Binary file or forced binary
            if(p->binary || (!(p->text) && (is_binary(files[0].f) || is_binary(files[1].f)))) {
                #ifdef DEBUG
                    printf("Binary files detected\n--------------\nStart of files comparison...\n");
                #endif

                if(diff_file_binary(files[0].f, files[1].f)) {
                    ret = EXIT_DIFFERENTS_FILES;
                    #ifdef DEBUG
                        printf("Binary files are different\n...comparison of files completed\n--------------\n");
                    #endif
                        printf("Binary files %s and %s differ\n", files[0].label, files[1].label);

                } else {
                    ret = EXIT_IDENTICAL_FILES;
                    #ifdef DEBUG
                        printf("Binary files are identical\n...comparison of files completed\n--------------\n");
                    #endif

                    if(p->report_identical_files)
                        printf("Binary files %s and %s are identical\n", files[0].label, files[1].label);
                }
            }
            // Text file or forced text
            else {

                #ifdef DEBUG
                    printf("Text files detected\n--------------\nStart of files indexing...\n");

                    START_TIMER;
                #endif

                index_file(&files[0]);
                index_file(&files[1]);

                #ifdef DEBUG
                    STOP_TIMER;
                    printf("...files indexing completed (%.4fs) \n--------------\n", GET_TIMER_VALUE);

                    if(p->d_show_index) {
                        index_display(&files[0]);
                        index_display(&files[1]);
                    }

                    if(p->d_interactive_mode) {
                        printf("Press enter to continue...\n");
                        getchar();
                    }
                #endif

                // Brief mode selected
                if(p->brief) {

                    #ifdef DEBUG
                        printf("Start of brief comparison...\n");
                    #endif

                    if(diff_file_brief(files[0].i, files[1].i)) {
                        ret = EXIT_DIFFERENTS_FILES;
                        #ifdef DEBUG
                            printf("Files are different\n...comparison of files completed\n--------------\n");
                        #endif

                        printf("Files %s and %s differ\n", files[0].label, files[1].label);
                    }
                    else {
                        ret = EXIT_IDENTICAL_FILES;
                        #ifdef DEBUG
                            printf("Files are identical\n...comparison of files completed\n--------------\n");
                        #endif

                        if(p->report_identical_files)
                            printf("Files %s and %s are identical\n", files[0].label, files[1].label);
                    }
                }
                // Regular mode
                else {
                    #ifdef DEBUG
                        printf("Start of files comparison...\n");

                        START_TIMER;
                    #endif

                    if(diff_file_regular(files)) {
                        if(p->in_recur)
                            print_args(files);

                        ret = EXIT_DIFFERENTS_FILES;

                        #ifdef DEBUG
                            STOP_TIMER;
                            printf("Files are different\n...comparison of files completed (%.4fs) \n--------------\n", GET_TIMER_VALUE);

                            if(p->d_show_diff) {
                                index_display(&files[0]);
                                index_display(&files[1]);
                            }

                            if(p->d_interactive_mode) {
                                printf("Press enter to continue...\n");
                                getchar();
                            }
                        #endif

                        print_diff(files);

                    } else {
                        ret = EXIT_IDENTICAL_FILES;

                        #ifdef DEBUG
                            STOP_TIMER;
                            printf("Files are identicals\n...comparison of files completed (%.4fs) \n--------------\n", GET_TIMER_VALUE);
                        #endif

                        if(p->o_style == COLUMNS) // Even if the files are identicals, show them
                            print_diff(files);

                        if(p->report_identical_files)
                            printf("Files %s and %s are identical\n", files[0].label, files[1].label);
                    }
                }
            }
            sec_fclose(files[1].f);

        } else
            ret = EXIT_ERROR;
        sec_fclose(files[0].f);

    } else
        ret = EXIT_ERROR;

    return ret;
}
