#include "err.h"

/* ===============================================
                     show_help

    Permet de signifier qu'on souhaite afficher la
    ligne
    'diff: Try 'diff --help' for more information'
    en plus de l'affichage classique de l'erreur.
    ----------------------------------------------
   =============================================== */
void show_help(void) {
    diff_stderr_show_help = 1;
}


/* ===============================================
                    send_error

    Permet de mettre à jour le buffer d'erreur. Si
    un des deux paramètre est null, alors le buffer
    correspondant sera vidé.
    Coupe également le programme en appellant 'exit'
    ----------------------------------------------
    const char* title : Titre de l'erreur
    const char* msg   : Contenu de l'erreur
   =============================================== */
void send_error(const char *title, const char* msg) {

    set_error(title, msg);

    exit(2);
}


/* ===============================================
                    set_error

    Permet de mettre à jour le buffer d'erreur. Si
    un des deux paramètre est null, alors le buffer
    correspondant sera vidé.
    ----------------------------------------------
    const char* title : Titre de l'erreur
    const char* msg   : Contenu de l'erreur
   =============================================== */
void set_error(const char *title, const char* msg) {

    diff_stderr = 1;

    if(title)
        strncpy(diff_stderr_title, title, 512);
    else
        diff_stderr_title[0] = '\0';

    if(msg)
        strncpy(diff_stderr_msg, msg, 1024);
    else
        diff_stderr_msg[0] = '\0';

}


/* ===============================================
                    diff_error

    Permet d'afficher le buffer d'erreur. Si le
    titre est vide (0) alors aucun titre ne sera
    affiché (Seul 'diff :' sera affiché).
    Si le message est vide, perror sera utilisé
    pour affiché l'erreur.

    Cette fonction est automatiquement appelée à
    la fin du programme.
    ----------------------------------------------
   =============================================== */
void diff_error(void) {

    if(diff_stderr) { // On a une erreur
        fprintf(stderr,"diff: ");

        if(diff_stderr_title[0] != 0) { // Si on a envoyé un titre

            if(diff_stderr_msg[0] != 0) { // Si on a envoyé un message
                fprintf(stderr,"%s: %s\n",diff_stderr_title, diff_stderr_msg);
            } else { // Sinon on utilise perror
                perror(diff_stderr_title);
            }

        } else {

            if(diff_stderr_msg[0] != 0) { // Si on a envoyé un message sans titre
                fprintf(stderr,"%s\n", diff_stderr_msg);
            } else { // Sinon on utilise perror
                perror(NULL);
            }

        }

        if(diff_stderr_show_help)
            fprintf(stderr, "diff: Try 'diff --help' for more information.");
    }

}
