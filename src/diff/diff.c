#include "diff.h"

/* Statics prototypes */
static _bool is_binary(FILE *f);

static _bool diff_file_binary(FILE *f1, FILE* f2);
static _bool diff_file_brief(Index *i1, Index *i2);

static void diff_analyse_index(File files[]);
static ulint diff_get_start(Index *i1, Index *i2);
static void diff_file_LCS(Smatrix **s, Index *i1, Index *i2, ulint start);
static _bool diff_file_regular(File files[]);

static const char* get_type_char(struct stat* s);
static int dir_search(const char* to_search, DIR* d);
static int diff_dir_make(File files[]);
static int diff_dir_make_r(File files[], struct  dirent* dr1, struct dirent* dr2);

/* ===============================================
                      sec_fopen

    Ouverture sécurisée, retour le résultat de
    fopen et affiche l'erreur s'il y en a une.
    ----------------------------------------------
    c char *path : chemin
    c char *mode : mode d'ouverture
    ----------------------------------------------
    Retourne le résultat de fopen avec path et
    mode.
   =============================================== */
FILE* sec_fopen(const char* path, const char* mode) {
    FILE* f = fopen(path, mode);

    if(!f)
        send_error(path, NULL, NULL);

    return f;
}


/* ===============================================
                      sec_fclose

    Fermeture sécurisée, affiche l'erreur s'il y
    en a une.
    ----------------------------------------------
    FILE *f : Flux à fermer
   =============================================== */
void sec_fclose(FILE *f) {

    if(f) {
        if(fclose(f) != 0)
            send_error("fclose()", NULL, NULL);
    }
}


/* ===============================================
                      sec_closedir

    Fermeture sécurisée, affiche l'erreur s'il y
    en a une.
    ----------------------------------------------
    DIR *f : Directory à fermer
   =============================================== */
void sec_closedir(DIR *d) {

    if(d) {
        if(closedir(d) == -1)
            send_error("closedir()", NULL, NULL);
    }
}


/* ===============================================
                      sec_opendir

    Ouverture sécurisée, retour le résultat de
    opendir et affiche l'erreur s'il y en a une.
    ----------------------------------------------
    c char *path : chemin
    ----------------------------------------------
    Retourne le résultat de opendir avec path.
   =============================================== */
DIR *sec_opendir(const char* path) {

    DIR *d = opendir(path);

    if(!d)
        send_error(path, NULL, NULL);

    return d;
}


/* ===============================================
                      free_diff

    Permet de libérer de la mémoire les éléments
    du diffs avant la fin du programme
    ----------------------------------------------
    files : tableau de structures File
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


void free_diff_r(File files[]) {

    sint i = 0;

    for(; i < 2; i++) {
        if(files[i].i)
            index_free(&files[i]);
    }
}


/* ===============================================
                    diff_file_brief

    Compare de manière rapide les deux indexs
    donnés en paramètre.
    ----------------------------------------------
    Index i1 : index n°1
    Index i2 : index n°2
    ----------------------------------------------
    Retourne true si les fichiers sont identiques,
    et false s'il sont différents
   =============================================== */
static _bool diff_file_brief(Index *i1, Index *i2) {
    ulint j = 0;

    /* If we have the same amount of lines */
    if(i1->line_max == i2->line_max) {
        /* Compare every lines */
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

    Permet d'essayer de deviner qu'un fichier est
    binaire en lisant les 4 premiers octets de
    celui-ci (Par convention, un fichier binaire
    commence par 4 fois la valeur '0').
    ----------------------------------------------
    FILE *f : fichier à tester
    ----------------------------------------------
    Retourne true si le fichiers est detecté
    comme binaire, false sinon.
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

    diff de deux fichiers binaire, permet de
    comparer directement le contenu des deux
    fichiers octets à octets sans se soucier de
    l'indexation.
    ----------------------------------------------
    FILE *f1 : fichier à tester n°1
    FILE *f2 : fichier à tester n°2
    ----------------------------------------------
    Retourne true si les fichiers sont identiques,
    false sinon.
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

    Permet de compter le nombre de lignes
    différentes dans un index à partir d'une ligne
    donnée.
    ----------------------------------------------
    Index *index : index à traiter
    ulint i      : début zone différente
    ----------------------------------------------
    Retourne le dernier index de la suite de
    modifications.
   =============================================== */
ulint diff_get_length(Index* index, ulint i) {

    ulint k = i;

    // We search the lenght of the change
    for(; k < index->line_max && index->lines[k].modified; k++);

    return k - i;
}


/* ===============================================
                 diff_analyse_index

    Permet d'analyser un fichier d'index pour en
    déduire si une ligne est ajoutée, supprimée
    ou encore modifiée. Cette opération est faite
    pour simplifier les traitements d'affichages.
    ----------------------------------------------
    Index *i1 : index à traiter n°1
    Index *i2 : index à traiter n°2
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
                 diff_get_start

    Permet de savoir où commence réellement la zone
    modifiée entres les deux fichiers indexés.
    Cette opération permet de limiter le nombre
    de tests du LCS.
    ----------------------------------------------
    Index *i1 : index à traiter n°1
    Index *i2 : index à traiter n°2
    ----------------------------------------------
    Retourne le premier indexe de la zone modifiée.
   =============================================== */
static ulint diff_get_start(Index *i1, Index *i2) {

    ulint i = 0;

    // We search the first change
    while(i < i1->line_max && i < i2->line_max && i1->lines[i].h == i2->lines[i].h)
        i++;

    return (i > 0 ? --i : 0);
}


/* ===============================================
                  diff_file_LCS

    Permet de faire le diff entre deux fichiers
    indexés, en stockant le résultat dans une
    matrice.
    ----------------------------------------------
    Smatrix **s : matrice
    Index *i1   : index à traiter n°1
    Index *i2   : index à traiter n°2
    ulint start : début du traitement
   =============================================== */
static void diff_file_LCS(Smatrix **s, Index *i1, Index *i2, ulint start) {

    ulint i = 0, j = 0, start_j = 0;
    _bool first_y = _true;

    i = start_j = start;

    *s = (Smatrix*)calloc(sizeof(Smatrix), i1->line_max);

    for(; i < i1->line_max; i++) {
        for(j = start_j; j < i2->line_max; j++, first_y = _true) {
            if(i1->lines[i].h == i2->lines[j].h || (i1->lines[i].ignore && i2->lines[j].ignore)) {
                smatrix_append(&(*s)[i], j);
                if(first_y) { // Optimization that make the algo focus on the first branch
                    start_j = j+1;
                    first_y = _false;
                }
            }
        }
    }

}

/* ===============================================
                  diff_file_regular

    Permet de faire le diff entre deux fichiers
    indexés.
    ----------------------------------------------
    File files[] : fichiers à traiter.
    ----------------------------------------------
    Retourne vrai si les fichiers sont identiques,
    et faux s'ils sont différents.
   =============================================== */
static _bool diff_file_regular(File files[]) {

    Smatrix *s = NULL;
    ulint ret = 0, start = diff_get_start(files[0].i, files[1].i);

    diff_file_LCS(&s, files[0].i, files[1].i, start);
    ret = smatrix_to_index(s, files[0].i, files[1].i, start);
    smatrix_free(s, files[0].i->line_max);

    if(files[0].i->line_max == files[1].i->line_max && files[0].i->line_max == ret) // If different
        return _false;
    else {
        diff_analyse_index(files);
        return _true;
    }
}


/* ===============================================
                 get_type_char

    Permet de récupérer facilement le nom du type
    d'un fichier à partir de son stat.
    ----------------------------------------------
    stat *s : structure à tester
    ----------------------------------------------
    Retourne le nom du type sous forme de chaine
    statique.
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

    Permet de localiser un nom de fichier dans une
    directory DIR.
    ----------------------------------------------
    const char* to_search : nom à chercher
    DIR* d                : directory où chercher
    ----------------------------------------------
    Retourne l'index du fichier ou -1 s'il ne peut
    le trouver.
   =============================================== */
static int dir_search(const char* to_search, DIR* d) {

    int d_save = telldir(d), d_val = 0;
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

static int diff_dir_make_r_new_file(File files[], struct  dirent* dr1, struct dirent* dr2) {

    File new_files[2];
    char *tmp_names[2] = {NULL, NULL};
    char *labels[2] = {NULL, NULL};
    struct stat st1, st2;
    int ret = EXIT_IDENTICAL_FILES;

    File_type type = T_NONE;

    if(!dr1) {
        tmp_names[0] = (char*)malloc(sizeof(char)*(diff_strlen(dr2->d_name)+diff_strlen(files[0].path)+2));
        sprintf(tmp_names[0], "%s/%s", files[0].path, dr2->d_name);
    } else {
        tmp_names[0] = (char*)malloc(sizeof(char)*(diff_strlen(dr1->d_name)+diff_strlen(files[0].path)+2));
        sprintf(tmp_names[0], "%s/%s", files[0].path, dr1->d_name);
    }

    if(!dr2) {
        tmp_names[1] = (char*)malloc(sizeof(char)*(diff_strlen(dr1->d_name)+diff_strlen(files[1].path)+2));
        sprintf(tmp_names[1], "%s/%s", files[1].path, dr1->d_name);
    } else {
        tmp_names[1] = (char*)malloc(sizeof(char)*(diff_strlen(dr2->d_name)+diff_strlen(files[1].path)+2));
        sprintf(tmp_names[1], "%s/%s", files[1].path, dr2->d_name);
    }


    if(!dr1) {
        if(stat(tmp_names[1], &st2) == 0) {
            if(p->recursive_dir && S_ISDIR(st2.st_mode)) {
                #ifdef DEBUG
                    printf("Directory forced as empty\n--------------\nStart of sub-directories comparison...\n");
                #endif
                type = T_DIR;
            } else if (S_ISREG(st2.st_mode)) {
                #ifdef DEBUG
                    printf("File forced as empty\n--------------\nStart of sub-files comparison...\n");
                #endif
                type = T_FILE;
            } else
                ret = EXIT_DIFFERENTS_FILES;
        } else {
            send_error(tmp_names[1], NULL);
            ret = EXIT_DIFFERENTS_FILES;
        }
    } else {
        if(stat(tmp_names[0], &st1) == 0) {
            if(p->recursive_dir && S_ISDIR(st1.st_mode)) {
                #ifdef DEBUG
                    printf("Directory forced as empty\n--------------\nStart of sub-directories comparison...\n");
                #endif
                type = T_DIR;
            } else if (S_ISREG(st1.st_mode)) {
                #ifdef DEBUG
                    printf("File forced as empty\n--------------\nStart of sub-files comparison...\n");
                #endif
                type = T_FILE;
            } else
                ret = EXIT_DIFFERENTS_FILES;
        } else {
            send_error(tmp_names[0], NULL);
            ret = EXIT_DIFFERENTS_FILES;
        }
    }


    if(ret == EXIT_IDENTICAL_FILES) {

        /* Labels to copy */
        labels[0] = ( files[0].path != files[0].label ? files[0].label : NULL );
        labels[1] = ( files[1].path != files[1].label ? files[1].label : NULL );

        init_diff_r(tmp_names, labels, new_files);

        if(type == T_DIR)
            ret = diff_dir(new_files);
        else {
            print_args(tmp_names);
            ret = diff_file(new_files);
        }

        free_diff_r(new_files);

        #ifdef DEBUG
            printf("End of comparison\n--------------\n");
        #endif
    }

    free(tmp_names[0]);
    free(tmp_names[1]);

    return ret;
}



static int diff_dir_make_r(File files[], struct  dirent* dr1, struct dirent* dr2) {

    File new_files[2];
    char *tmp_names[2] = {NULL, NULL};
    char *labels[2] = {NULL, NULL};
    struct stat st1, st2;
    int ret = EXIT_IDENTICAL_FILES;

    File_type type = T_NONE;

    tmp_names[0] = (char*)malloc(sizeof(char)*(diff_strlen(dr1->d_name)+diff_strlen(files[0].path)+2));
    sprintf(tmp_names[0], "%s/%s", files[0].path, dr1->d_name);

    tmp_names[1] = (char*)malloc(sizeof(char)*(diff_strlen(dr2->d_name)+diff_strlen(files[1].path)+2));
    sprintf(tmp_names[1], "%s/%s", files[1].path, dr2->d_name);

    if(stat(tmp_names[0], &st1) == 0) {
        if(stat(tmp_names[1], &st2) == 0) {
            if(st1.st_mode == st2.st_mode) {
                if(p->recursive_dir && S_ISDIR(st1.st_mode)) {
                    type = T_DIR;
                    #ifdef DEBUG
                        printf("Two sub-directoriers detected\n--------------\nStart of sub-directories comparison...\n");
                    #endif
                } else if (S_ISREG(st1.st_mode)) {
                    type = T_FILE;
                    #ifdef DEBUG
                        printf("Two sub-files detected\n--------------\nStart of sub-files comparison...\n");
                    #endif
                }
                /* Sinon ... */
                else {
                    printf("File %s is a %s while file %s is a %s\n",
                            tmp_names[0], get_type_char(&st1), tmp_names[1], get_type_char(&st2));
                    ret = EXIT_IDENTICAL_FILES;
                }
            } else
                ret = EXIT_DIFFERENTS_FILES;
        } else {
            send_error(tmp_names[1], NULL);
            ret = EXIT_DIFFERENTS_FILES;
        }
    } else {
        send_error(tmp_names[0], NULL);
        ret = EXIT_DIFFERENTS_FILES;
    }

    if(ret == EXIT_IDENTICAL_FILES) {

        // Labels to copy
        labels[0] = ( files[0].path != files[0].label ? files[0].label : NULL );
        labels[1] = ( files[1].path != files[1].label ? files[1].label : NULL );

        p->new_file = _true;
        init_diff_r(tmp_names, labels, new_files);

        if(type == T_DIR)
            ret = diff_dir(new_files);
        else {
            print_args(tmp_names);
            ret = diff_file(new_files);
        }

        free_diff_r(new_files);
        p->new_file = _false;

        #ifdef DEBUG
            printf("End of comparison\n--------------\n");
        #endif
    }

    free(tmp_names[0]);
    free(tmp_names[1]);

    return ret;
}


/* ===============================================
                    diff_dir_make

    Permet de réaliser le diff entre deux dossiers
    en testant les éléments qu'il contient.

    Renvoit 0 si leur contenu est identique, 1
    sinon.
    ----------------------------------------------

    ----------------------------------------------

   =============================================== */
static int diff_dir_make(File files[]) {

    int ret = EXIT_IDENTICAL_FILES, tmp = 0, i = 0;
    struct dirent *dr1 = NULL, *dr2 = NULL;

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
    while (!files[0].empty && (dr1 = readdir(files[0].d)) != NULL) {
        if(!files[1].empty && (dr2 = readdir(files[1].d)) != NULL) {
            // Same name
            if((!p->ignore_case_filename && diff_strcmp(dr1->d_name, dr2->d_name) == 0) ||
                diff_strcasecmp(dr1->d_name, dr2->d_name) == 0)
            {
                tmp = diff_dir_make_r(files, dr1, dr2);
                if (tmp > ret)
                    ret = tmp;
            }
            // Different name
            else {
                // d1 is further in d2
                if((tmp = dir_search(dr1->d_name, files[1].d)) != -1) {
                    // Every file in d2 until tmp aren't in d1
                    do {
                        ret = ( p->new_file_recur ? ((tmp = diff_dir_make_r_new_file(files, dr1, dr2)) > ret ? tmp : ret ) :
                               EXIT_DIFFERENTS_FILES, printf("Only in %s: %s\n", files[1].path, dr2->d_name));
                    } while((dr2 = readdir(files[1].d)) != NULL && telldir(files[1].d) < tmp);
                    seekdir(files[0].d, telldir(files[0].d)-1);
                    seekdir(files[1].d, telldir(files[1].d)-1);
                }
                // d2 is further in d1
                else if((tmp = dir_search(dr2->d_name, files[0].d)) != -1) {
                    // Every file in d1 until tmp arent in d2
                    do {
                        ret = ( p->new_file_recur ? ((tmp = diff_dir_make_r_new_file(files, dr1, dr2)) > ret ? tmp : ret ) :
                               EXIT_DIFFERENTS_FILES, printf("Only in %s: %s\n", files[0].path, dr1->d_name));
                    } while((dr1 = readdir(files[0].d)) != NULL && telldir(files[0].d) < tmp);
                    seekdir(files[0].d, telldir(files[0].d)-1);
                    seekdir(files[1].d, telldir(files[1].d)-1);
                }
                // d2 is not further in d1 and vice versa
                else {
                    printf("Only in %s: %s\n", files[0].path, dr1->d_name);
                    printf("Only in %s: %s\n", files[1].path, dr2->d_name);
                    ret = EXIT_DIFFERENTS_FILES;
                }
            }
        } else {
            ret = ( p->new_file_recur ? ((tmp = diff_dir_make_r_new_file(files, dr1, dr2)) > ret ? tmp : ret ) :
                   EXIT_DIFFERENTS_FILES, printf("Only in %s: %s\n", files[0].path, dr1->d_name));
        }
    }

    // Only in d2
    while((dr2 = readdir(files[1].d)) != NULL) {
        ret = ( p->new_file_recur ? ((tmp = diff_dir_make_r_new_file(files, dr1, dr2)) > ret ? tmp : ret ) :
                EXIT_DIFFERENTS_FILES, printf("Only in %s: %s\n", files[1].path, dr2->d_name));
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

int diff_dir(File files[]) {

    int ret = EXIT_IDENTICAL_FILES;

    if(files[0].empty || (files[0].d = sec_opendir(files[0].path))) {
        if(files[1].empty || (files[1].d = sec_opendir(files[1].path))) {
            ret = diff_dir_make(files);
            sec_closedir(files[0].d);
            sec_closedir(files[1].d);
        } else {
            sec_closedir(files[0].d);
            ret = EXIT_ERROR;
        }

    } else
        ret = EXIT_ERROR;

    return ret;
}


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
