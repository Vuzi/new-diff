#include "diff.h"

/* Prototype des statiques */
static _bool is_binary(FILE *f);

static _bool diff_file_binary(FILE *f1, FILE* f2);
static _bool diff_file_brief(Index *i1, Index *i2);

static void diff_analyse_index(Index *i1, Index *i2);
static ulint diff_get_start(Index *i1, Index *i2);
static void diff_file_LCS(Smatrix **s, Index *i1, Index *i2, ulint start);
static _bool diff_file_regular(File files[]);

static const char* get_type_char(struct stat* s);
static int dir_search(const char* to_search, DIR* d);
static int diff_dir_make(const char* d1_name, DIR* d1, const char* d2_name, DIR* d2);


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

    if(!f) {
        send_error(path, NULL, NULL);
    }

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

    if(fclose(f) != 0) {
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

    if(closedir(d) == -1) {
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

    if(!d) {
        send_error(path, NULL, NULL);
    }

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

    /* Si nombre de lignes identiques */
    if(i1->line_max == i2->line_max) {
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

    int i = 0, c = getc(f);

    while(i < 4 && c == (int)'\0' && c != EOF) {
        i++;
        c = getc(f);
    }

    rewind(f);

    if(i < 4)
        return _false;
    else
        return _true;

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

    fseek(f1, 0, SEEK_END);
    fseek(f2, 0, SEEK_END);

    /* Si même taille */
    if(ftell(f1) == ftell(f2)) {

        rewind(f1);
        rewind(f2);

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

    /* On regarde jusqu'où va la zone de changements */
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
static void diff_analyse_index(Index *i1, Index *i2) {

    ulint i = 0, j = 0, k = 0;

    ulint length_1 = 0, length_2 = 0;

    /* Pour chaque ligne */
    /* Fonctionne car si les lignes ajoutées sont après la fin, la dernière ligne de l'autre fichier sera modifiée */
    for(; i < i1->line_max && j < i2->line_max; i++, j++) {

        /* Une des deux lignes modifiée */
        if(i1->lines[i].modified || i2->lines[j].modified) {

            length_1 = diff_get_length(i1, i);
            length_2 = diff_get_length(i2, j);

            /* Deletions */
            if(length_2 == 0) {
                for(k = i; k < i+length_1; k++)
                    i1->lines[k].modified = LINE_DEL;
            }
            /* Additions */
            else if (length_1 == 0) {
                for(k = j; k < j+length_2; k++)
                    i2->lines[k].modified = LINE_ADD;
            }
            /* Changements */
            else {
                for(k = i; k < i+length_1; k++)
                    i1->lines[k].modified = LINE_CHANGE;
                for(k = j; k < j+length_2; k++)
                    i2->lines[k].modified = LINE_CHANGE;
            }

            /* On reprend après */
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

    while(i < i1->line_max && i < i2->line_max && i1->lines[i].h == i2->lines[i].h)
        i++;

    if(i > 0)
        return --i;
    else
        return 0;
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
            if(i1->lines[i].h == i2->lines[j].h) {
                smatrix_append(&(*s)[i], j);
                if(first_y) {
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
    ulint ret = 0, start =  diff_get_start(files[0].i, files[1].i);

    diff_file_LCS(&s, files[0].i, files[1].i, start);
    ret = smatrix_to_index(s, files[0].i, files[1].i, start);
    smatrix_free(s, files[0].i->line_max);

    if(files[0].i->line_max == files[1].i->line_max && files[0].i->line_max == ret)
        return _false;
    else {
        diff_analyse_index(files[0].i, files[1].i);
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
        if(strcmp(to_search, dr->d_name) == 0) {
            d_val = telldir(d);
            seekdir(d, d_save);
            return d_val;
        }
    }

    seekdir(d, d_save);
    return -1;
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
static int diff_dir_make(const char* d1_name, DIR* d1, const char* d2_name, DIR* d2) {

    int ret = EXIT_IDENTICAL_FILES, tmp = 0;
    char *tmp_name1 = NULL, *tmp_name2 = NULL;

    struct dirent *dr1 = NULL, *dr2 = NULL;
    struct stat st1, st2;

    #ifdef DEBUG
        printf("Start of directories comparison...\n");

        START_TIMER;
    #endif

    /* On passe le . & .. */
    seekdir(d1, 2);
    seekdir(d2, 2);

    /* Pour chaque dossier dans d1 */
    while ((dr1 = readdir(d1)) != NULL) {
        if((dr2 = readdir(d2)) != NULL) {
            /* S'il s'agit du même */
            if(strcmp(dr1->d_name, dr2->d_name) == 0) {

                tmp_name1 = (char*)malloc(sizeof(char)*(strlen(dr1->d_name)+strlen(d1_name)+2));
                sprintf(tmp_name1, "%s/%s", d1_name, dr1->d_name);

                tmp_name2 = (char*)malloc(sizeof(char)*(strlen(dr2->d_name)+strlen(d2_name)+2));
                sprintf(tmp_name2, "%s/%s", d2_name, dr2->d_name);

                /* On récupère le type */
                if(stat(tmp_name1, &st1) == 0) {
                    if(stat(tmp_name2, &st2) == 0) {
                        /* S'il sont du même type */
                        if(st1.st_mode == st2.st_mode) {
                            /* A voir/ faire
                                Si dossier ET -r ou si fichiers
                                1. Afficher la commande
                                2. Soit rappeller le main, ou relancer directement le programme */
                        }
                        /* Sinon ... */
                        else {
                            printf("File %s/%s is a %s while file %s/%s is a %s\n",
                                    d1_name, dr1->d_name, get_type_char(&st1), d2_name, dr2->d_name, get_type_char(&st2));
                            ret = EXIT_DIFFERENTS_FILES;
                        }
                    } else
                        send_error(tmp_name2, NULL);
                } else
                    send_error(tmp_name1, NULL);

                free(tmp_name1);
                free(tmp_name2);
            }
            /* Il ne s'agit pas du même */
            else {
                /* On retrouve d1 plus loin dans d2 */
                if((tmp = dir_search(dr1->d_name, d2)) != -1) {
                    /* Tout les elements de d2 jusqu'à tmp ne sont que dans d2 */
                    do {
                        printf("Only in %s: %s\n", d2_name, dr2->d_name);
                        ret = EXIT_DIFFERENTS_FILES;
                    } while((dr2 = readdir(d2)) != NULL && telldir(d2) < tmp);

                }
                /* On retrouve d2 plus loin dans d1 */
                else if((tmp = dir_search(dr2->d_name, d1)) != -1) {
                    /* Tout les elements de d1 jusqu'à tmp ne sont que dans d1 */
                    do {
                        printf("Only in %s: %s\n", d1_name, dr1->d_name);
                        ret = EXIT_DIFFERENTS_FILES;
                    } while((dr1 = readdir(d1)) != NULL && telldir(d1) < tmp);
                }
                else {
                    printf("Only in %s: %s\n", d1_name, dr1->d_name);
                    printf("Only in %s: %s\n", d2_name, dr2->d_name);
                    ret = EXIT_DIFFERENTS_FILES;
                }
            }
        } else {
            printf("Only in %s: %s\n", d1_name, dr1->d_name);
            ret = EXIT_DIFFERENTS_FILES;
        }
    }

    /* Fichier dans d2 en plus */
    while((dr2 = readdir(d2)) != NULL) {
        printf("Only in %s: %s\n", d2_name, dr2->d_name);
        ret = EXIT_DIFFERENTS_FILES;
    }


    #ifdef DEBUG
        STOP_TIMER;
        if(ret == EXIT_DIFFERENTS_FILES)
            printf("Directories are different\n...comparison of directories completed (%.4fs) \n--------------\n", GET_TIMER_VALUE);
        else
            printf("Directories are identical\n...comparison of directories completed (%.4fs) \n--------------\n", GET_TIMER_VALUE);
    #endif

    return ret;
}

int diff_dir(File files[]) {

    int ret = EXIT_IDENTICAL_FILES;

    DIR *d1 = sec_opendir(files[0].path), *d2 = sec_opendir(files[1].path);

    if(d1) {
        if(d2) {
            ret = diff_dir_make(files[0].path, d1, files[1].path, d2);
            sec_closedir(d1);
            sec_closedir(d2);
        } else {
            sec_closedir(d1);
            ret = EXIT_ERROR;
        }

    } else
        ret = EXIT_ERROR;

    return ret;
}


int diff_file(File files[]) {

    int ret = EXIT_IDENTICAL_FILES;

    files[0].f = sec_fopen(files[0].path, "rb+");
    files[1].f = sec_fopen(files[1].path, "rb+");

    if(files[0].f) {
        if(files[1].f) {

            /* Fichier binaire ou forcé binaire */
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
            /* Fichier texte ou forcé texte */
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
                        if(p->d_interactive_mode) {
                            printf("Press enter to continue...\n");
                            getchar();
                        }
                    }
                #endif

                /* Mode brief selectionné */
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
                /* Mode régulier */
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
                        #endif

                        print_diff(files);

                    } else {
                        ret = EXIT_IDENTICAL_FILES;

                        #ifdef DEBUG
                            STOP_TIMER;
                            printf("Files are identicals\n...comparison of files completed (%.4fs) \n--------------\n", GET_TIMER_VALUE);
                        #endif

                        if(p->o_style == COLUMNS) // Même si le fichier est identique, il faut l'afficher
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
