#ifndef INDEX_H_INCLUDED
#define INDEX_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define CREATE_PARAMETERS

#include "../constant.h"

#define END_LINE '\n'

/* Structure de parcours des fichiers */
struct s_index {

    FILE* f;

    char* f_name;

    /* Cas des fichiers >= 1Mo */
    unsigned int *index;   // Fin de chaque ligne

    /* Cas des fichiers < 1Mo */
    char **lines;

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
