#include "diff.h"

/* Prototype des statiques */
static _bool is_binary(FILE *f);

static _bool diff_file_binary(FILE *f1, FILE* f2);
static _bool diff_file_brief(Index *i1, Index *i2);

static uint diff_get_start(Index *i1, Index *i2);
static void diff_file_LCS(Subsq **s, Index *i1, Index *i2, uint start);
static _bool diff_file_regular(File file_1, File file_2);

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

static _bool diff_file_brief(Index *i1, Index *i2) {
    uint j = 0;

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

static uint diff_get_start(Index *i1, Index *i2) {

    uint i = 0;

    while(i < i1->line_max && i < i2->line_max && i1->lines[i].h == i2->lines[i].h)
        i++;

    if(i > 0)
        return --i;
    else
        return 0;
}

static void diff_file_LCS(Subsq **s, Index *i1, Index *i2, uint start) {

    uint i = 0, j = 0, start_j = 0;
    _bool first_y = _true;

    i = start_j = start;

    *s = (Subsq*)calloc(sizeof(Subsq), i1->line_max);

    for(; i < i1->line_max; i++) {
        for(j = start_j; j < i2->line_max; j++, first_y = _true) {
            if(i1->lines[i].h == i2->lines[j].h) {
                subsq_append(&(*s)[i], j);
                if(first_y) {
                    start_j = j+1;
                    first_y = _false;
                }
            }
        }
    }

}

static _bool diff_file_regular(File file_1, File file_2) {

    Subsq *s = NULL;
    uint ret = 0, start =  diff_get_start(file_1.i, file_2.i);

    diff_file_LCS(&s, file_1.i, file_2.i, start);
    ret = subsq_to_index(s, file_1.i, file_2.i, start);
    subsq_free(s, file_1.i->line_max);

    if(file_1.i->line_max == file_2.i->line_max && file_1.i->line_max == ret)
        return _false;
    else
        return _true;
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
                #endif

            #include "time.h"
            time_t start,end;
            start=clock();

                index_file(&files[0]);
                index_file(&files[1]);

            end=clock();
            printf("Timer : %.4fs\n", (end-start)/(double)CLOCKS_PER_SEC);

                #ifdef DEBUG
                    printf("...files indexing completed\n--------------\n");

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
                    #endif
            //time_t start,end;
            start=clock();
                    if(diff_file_regular(files[0], files[1])) {
                        ret = EXIT_DIFFERENTS_FILES;

                        #ifdef DEBUG
                            printf("Files are different\n...comparison of files completed\n--------------\n");
                        #endif

                        print_diff(files);

                    } else {
                        ret = EXIT_IDENTICAL_FILES;

                        #ifdef DEBUG
                            printf("Files are identicals\n...comparison of files completed\n--------------\n");
                        #endif

                        if(p->o_style == COLUMNS) // Même si le fichier est identique, il faut l'afficher
                            print_diff(files);

                        if(p->report_identical_files)
                            printf("Files %s and %s are identical\n", files[0].label, files[1].label);
                    }
                    end=clock();
            printf("Timer : %.4fs\n\n", (end-start)/(double)CLOCKS_PER_SEC);

                    index_display(&files[0]);
                    index_display(&files[1]);
                }

                /* Free de l'indexe */
                index_free(files);

            }
            sec_fclose(files[1].f);

        } else
            ret = EXIT_ERROR;
        sec_fclose(files[0].f);

    } else
        ret = EXIT_ERROR;

    return ret;
}
