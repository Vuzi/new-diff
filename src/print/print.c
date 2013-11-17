#include "print.h"

/* Statiques */
static void print_diff_regular(File f1, File f2);
static void print_space(unsigned int n);
void print_lines(File file, uint start, uint end, const char *line_start);
void print_lines_lenght(File file, unsigned int start, unsigned int end, const char *line_start, unsigned int lenght);

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
void print_lines(File file, uint start, uint end, const char *line_start) {
    print_lines_lenght(file, start, end, line_start, 0);
}

/* ===============================================
                lines_display_lenght

    Affiche les lignes de f de start à end (compris)
    en ajoutant line_start avant chaque ligne avec
    un maximum de lenght caractères. 0 permet de ne
    pas utiliser de limitation.
    Saute une ligne après chaque ligne écrite.
    ----------------------------------------------
    t_index *f       : fichier où sont les lignes.
    u int start      : première ligne.
    u int end        : dernière ligne.
    char* line_start : à afficher avant chaque ligne
    u int lenght     : nombre de char max
   =============================================== */
void print_lines_lenght(File file, uint start, uint end, const char *line_start, uint lenght) {

    uint i = 0, j = 0, l = 0;
    int c = 0;

    if(start <= end && end <= file.i->line_max) { // Si taille possible
        for(i = start; i <= end; i++) { // Pour chacune des lignes à afficher

            if(line_start)
                fputs(line_start, stdout);

            if(file.i->lines[i].lenght > 0 ) {
                l = (lenght > 0 && !(lenght > 0 && lenght < file.i->lines[i].lenght) ? lenght : file.i->lines[i].lenght);

                fseek(file.f, (long)file.i->lines[i].start, SEEK_SET);
                for(j = 0; j < l; j++) {
                    if((c = getc(file.f)) == '\t')
                        print_space(p->size_tab);
                    else
                        putchar(c);
                }
            }

            putchar('\n');
        }
    }
}

/* ===============================================
                    print_space

    Permet d'afficher simplement un nombre donné
    d'espace sur la sortie standard
    ----------------------------------------------
    u int n : nombre d'espaces
   =============================================== */
static void print_space(unsigned int n) {
    unsigned int i = 0;

    for(i = 0; i < n; i++)
        fputc(' ', stdout);
}


void print_diff(File files[]) {

    #ifdef DEBUG
        printf("Start of the result display...\n");
    #endif

    /* Format de sortie contextuel */
    if(p->o_style == CONTEXT)
        //diff_display_context(diff, f1, f2);
        puts("A faire");
    /* Format de sortie unifé */
    else if(p->o_style == UNIFIED)
        //diff_display_unified(diff, f1, f2);
        puts("A faire");
    /* Format edit script */
    else if(p->o_style == EDIT_SCRIPT)
        //diff_display_ed(diff, f2);
        puts("A faire");
    /* Format en colonnes */
    else if(p->o_style == COLUMNS)
        //diff_display_columns(diff, f1, f2, p->show_max_char);
        puts("A faire");
    /* Format pas défaut */
    else
        print_diff_regular(files[0], files[1]);

    #ifdef DEBUG
        printf("...display completed\n--------------\n");
    #endif

}

static void print_diff_regular(File f1, File f2) {

    uint i = 0, j = 0, k = 0;

    uint lenght_1 = 0, lenght_2 = 0;
    uint start_1 = 0, start_2 = 0;

    /* Pour chaque ligne */
    for(; i < f1.i->line_max && j < f2.i->line_max; i++, j++) {

        /* Une des deux lignes modifiée */
        if(f1.i->lines[i].modified || f2.i->lines[j].modified) {

            /* Màj valeurs */
            lenght_1 = (f1.i->lines[i].modified ? 1 : 0);
            lenght_2 = (f2.i->lines[j].modified ? 1 : 0);

            start_1 = (f1.i->lines[i].modified ? i : 0);
            start_2 = (f2.i->lines[j].modified ? j : 0);

            /* On regarde jusqu'où va la zone de changements */
            for(k = i+1; k < f1.i->line_max && f1.i->lines[k].modified; k++) {
                if(lenght_1 == 0)
                    start_1 = k;
                lenght_1++;
            }

            for(k = j+1; k < f2.i->line_max && f2.i->lines[k].modified; k++) {
                if(lenght_2 == 0)
                    start_2 = k;
                lenght_2++;
            }

            /* Deletions */
            if(lenght_2 == 0) {
                if(lenght_1 == 1)
                    printf("%ud%u\n", start_1+1, j);
                else
                    printf("%u,%ud%u\n", start_1+1, lenght_1 + start_1, j);

                print_lines(f1, start_1, lenght_1 + start_1 - 1, "< ");
            }
            /* Additions */
            else if (lenght_1 == 0) {
                if(lenght_2 == 1)
                    printf("%ua%u\n",i , start_2+1);
                else
                    printf("%ua%u,%u\n",i,  start_2+1, lenght_2 + start_2);

                print_lines(f2, start_2, lenght_2 + start_2 - 1, "> ");
            }
            /* Changements */
            else {
                if(lenght_1 == 1)
                    printf("%uc", start_1+1);
                else
                    printf("%u,%uc", start_1+1, lenght_1 + start_1);

                if(lenght_2 == 1)
                    printf("%u\n", start_2+1);
                else
                    printf("%u,%u\n", start_2+1, lenght_2 + start_2);

                print_lines(f1, start_1, lenght_1 + start_1 - 1, "< ");
                fputs("---\n", stdout);
                print_lines(f2, start_2, lenght_2 + start_2 - 1, "> ");
            }

            /* On reprend après */
            i += lenght_1;
            j += lenght_2;
        }
    }


}
