#include "index.h"

/* ===============================================
                    index_size

    Permet de compter le nombre de lignes du
    fichier f.
    ----------------------------------------------
    FILE *f : fichier à compter.
    ----------------------------------------------
    Retour : Retourne le nombre de lignes de f
   =============================================== */
unsigned int index_size(FILE *f) {
    unsigned int cpt = 0;
    char c = 0;

    while((c = getc(f)) != EOF) {
        if(c == END_LINE)
            cpt++;
    }
    cpt++; // Dernière ligne

    rewind(f);

    return cpt;
}

/* ===============================================
                    index_file

    Permet d'indexer un fichier pour permettre
    ensuite facilement le traitement.
    ----------------------------------------------
    FILE *f : fichier à indexer.
    ----------------------------------------------
    Retour : Retourne la structure instanciée pour
             l'nedexation. Renvoit une structure
             avec 0 ligne si le fichier n'est pas
             ouvert ou vide.
   =============================================== */
t_index* index_file(FILE *f) {

    /* On initialise */
    t_index *new_f = (t_index*)malloc(sizeof(t_index));

    char tmp = 0;
    unsigned int i = 0;

    /* Valeurs de base */
    new_f->f = f;
    new_f->line_max = index_size(new_f->f);
    new_f->index = (unsigned int*)malloc(sizeof(int)*new_f->line_max);
    new_f->line = 0;

    /* Rapide test pour vérifier si le fichier n'est pas vide */
    if(f && ((tmp = getc(f)) != EOF)) {

        rewind(f);

        /* Premiere ligne */
        new_f->index[i] = 0;
        i++;

        /* Pour chaque char */
        while((tmp = getc(f)) != EOF) {
            /* Si nouvelle ligne */
            if(tmp == END_LINE) {
                /* On rempli la case */
                new_f->index[i] = ftell(new_f->f);
                i++;
            }
        }

    }

    rewind(f);

    /* On renvoit la structure */
    return new_f;
}

/* ===============================================
                    free_index

    Permet de libérer un index.
    ----------------------------------------------
    t_index* index : index à libérer.
   =============================================== */
void index_free(t_index* index) {
    if(index) {
        free(index->index);
        free(index);
    }
}

/* ===============================================
                  display_index_file

    Affiche le contenu de la structure de f.
    ----------------------------------------------
    t_index *f : fichier à afficher.
   =============================================== */
void index_display(t_index *f) {

    unsigned int i = 0;

    if(f){
        printf("Fichier de %d ligne(s) :\n", f->line_max);
    }

    for(i = 0; i+1 < f->line_max; i++) {
        printf("Ligne %d : %d -> %d\n", i, f->index[i], f->index[i+1]-1);
    }
    printf("Ligne %d : %d -> EOF\n", i, f->index[i]);
}
