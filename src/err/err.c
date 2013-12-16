#include "err.h"


/* ===============================================
                    exit_error

    Print a formated error to stderr, and quit the
    program.
    msg should be formated as a "printf" follows
    by its values.
    If title is null, then no title will be
    displayed. If msg is null, perror will be used
    to print the error.
    ----------------------------------------------
    const char* title : Error title
    const char* msg   : Error content
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

    Print a formated error to stderr.
    msg should be formated as a "printf" follows
    by its values.
    If title is null, then no title will be
    displayed. If msg is null, perror will be used
    to print the error.
    ----------------------------------------------
    const char* title : Error title
    const char* msg   : Error content
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

    Activate the line
    "diff: Try 'diff --help' for more information."
    after displaying an error.
    ----------------------------------------------
   =============================================== */
void exit_help(void) {
        diff_stderr_show_help = 1;
}
