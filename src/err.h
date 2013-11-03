#ifndef H_ERR
#define H_ERR

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdarg.h>

#define CREATE_PARAMETERS

#include "constant.h"

/* Defines */

/* ===============================================
                    exit_error

    Permet d'afficher une erreur et de quitter le
    programme.
    msg doit être formaté comme un "printf"
    classique suivit de ses valeurs.
    ----------------------------------------------
    const char* title : Titre de l'erreur
    const char* msg   : Contenu de l'erreur
   =============================================== */
#define exit_error(title, msg, ...); \
            send_error(title, msg, __VA_ARGS__); \
            exit(2);


/* ===============================================
                    send_error

    Permet d'afficher une erreur.
    msg doit être formaté comme un "printf"
    classique suivit de ses valeurs.
    ----------------------------------------------
    const char* title : Titre de l'erreur
    const char* msg   : Contenu de l'erreur
   =============================================== */
#define send_error(title, msg, ...); \
            fprintf(stderr,"diff: "); \
            if( title != NULL ) { \
                fprintf(stderr,"%s: ", title); \
            } \
            if( msg != NULL ) { \
                fprintf(stderr, msg, __VA_ARGS__); \
                fputs("\n", stderr); \
            } else { \
                perror(NULL); \
            } \
            if(diff_stderr_show_help) { \
                fprintf(stderr, "diff: Try 'diff --help' for more information.\n"); \
            }

/* ===============================================
                    exit_help

    Permet d'activer l'affichage de la ligne
    "diff: Try 'diff --help' for more information."
    après l'affichage de l'erreur.
    ----------------------------------------------
   =============================================== */
#define exit_help(); \
        diff_stderr_show_help = 1;

#endif // H_ERR
