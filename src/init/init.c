#include "init.h"

#ifdef DEBUG
    #include "time.h"
    time_t debug_start_timer, debug_end_timer, debug_start_timer2, debug_end_timer2;
#endif

void init_diff(int argc, char** argv, File files[]) {

    uint i = 0;

    /* Analyse des paramètres */
    #ifdef DEBUG
        START_TIMER;
        printf("Start of the options analysing...\n");
    #endif

    initialize_params();
	atexit(free_params_glob); // Ajout à la list
    make_params(argc, argv);

    p->argc = argc;
    p->argv = argv;

    #ifdef DEBUG
        STOP_TIMER;
        printf("... options analysing completed (%.4fs) \n--------------\n", GET_TIMER_VALUE);

        if(p->d_show_options) {
            print_params(p);
        }
        if(p->d_interactive_mode) {
            printf("Press enter to continue...\n");
            getchar();
        }
    #endif

    for(; i < 2; i++) {
        files[i].empty = _false;

        /* Analyse des chemins */
        files[i].path = malloc(sizeof(char)*(strlen(p->paths[i])+1));
        strcpy(files[i].path, p->paths[i]);

        /* Labels */
        if(p->labels[i]) {
            files[i].label = malloc(sizeof(char)*(strlen(p->labels[i])+1));
            strcpy(files[i].label, p->labels[i]);
        } else {
            files[i].label = files[i].path;
        }

        /* Index */
        files[i].i = NULL;
        files[i].f = NULL;
    }

    set_paths(files);
}


void init_diff_r(char* paths[], char* labels[], File files[]) {

    uint i = 0;

    for(; i < 2; i++) {
        files[i].path = paths[i];
        files[i].label = ( labels[i] ? labels[i] : paths[i] );
        files[i].empty = _false;
        files[i].i = NULL;
        files[i].f = NULL;
    }

    set_paths(files);
}
