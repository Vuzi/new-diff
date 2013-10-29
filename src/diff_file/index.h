#ifndef INDEX_H_INCLUDED
#define INDEX_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CREATE_PARAMETERS

#include "../params/constant.h"

#define END_LINE '\n'

/* Structure de parcours des fichiers */
struct s_index {

    FILE* f;

    char* f_name;

    unsigned int *index;   // Fin de chaque ligne
    unsigned int line;     // Ligne actuelle
    unsigned int line_max; // Nombre de lignes

    unsigned int *c_func;
    unsigned int c_func_nb;

};

typedef struct s_index t_index;

/* Prototypes */
t_index* index_file(FILE *f, const char* f_name);
void index_file_c_func(t_index* index);
void index_free(t_index* index);
void index_display(t_index *f);

#endif // INDEX_H_INCLUDED
