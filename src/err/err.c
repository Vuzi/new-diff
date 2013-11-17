#include "err.h"


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
void exit_error(const char* title, const char* msg, ...) {

    va_list args;
    va_start(args, msg);

    fprintf(stderr,"diff: ");

    if( title != NULL ) {
        fprintf(stderr,"%s: ", title);
    }

    if( msg != NULL ) {
        vfprintf(stderr, msg, args);
        fputs("\n", stderr);
    } else {
        perror(NULL);
    }

    if(diff_stderr_show_help) {
        fprintf(stderr, "diff: Try 'diff --help' for more information.\n");
    }

    va_end(args);

    exit(EXIT_ERROR);
}


/* ===============================================
                    send_error

    Permet d'afficher une erreur.
    msg doit être formaté comme un "printf"
    classique suivit de ses valeurs.
    ----------------------------------------------
    const char* title : Titre de l'erreur
    const char* msg   : Contenu de l'erreur
   =============================================== */
void send_error(const char* title, const char* msg, ...) {

    va_list args;
    va_start(args, msg);

    fprintf(stderr,"diff: ");

    if( title != NULL ) {
        fprintf(stderr,"%s: ", title);
    }

    if( msg != NULL ) {
        vfprintf(stderr, msg, args);
        fputs("\n", stderr);
    } else {
        perror(NULL);
    }

    if(diff_stderr_show_help) {
        fprintf(stderr, "diff: Try 'diff --help' for more information.\n");
    }

    va_end(args);
}

/* ===============================================
                    exit_help

    Permet d'activer l'affichage de la ligne
    "diff: Try 'diff --help' for more information."
    après l'affichage de l'erreur.
    ----------------------------------------------
   =============================================== */
void exit_help(void) {
        diff_stderr_show_help = 1;
}
