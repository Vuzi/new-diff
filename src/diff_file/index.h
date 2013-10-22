#ifndef INDEX_H_INCLUDED
#define INDEX_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

#define END_LINE '\n'

/* Structure de parcours des fichiers */
struct s_index {

    FILE* f;
    unsigned int *index;   // Fin de chaque ligne
    unsigned int line;     // Ligne actuelle
    unsigned int line_max; // Nombre de lignes

};

typedef struct s_index t_index;

/* Prototypes */
unsigned int index_size(FILE *f);
t_index* index_file(FILE *f);
void index_free(t_index* index);
void index_display(t_index *f);

#endif // INDEX_H_INCLUDED
