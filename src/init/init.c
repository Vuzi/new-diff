#include "init.h"

#ifdef DEBUG
    #include "time.h"
    time_t debug_start_timer, debug_end_timer, debug_start_timer2, debug_end_timer2;
#endif


/* ===============================================
                     init_diff

    Initialize files with the arguments in argv.
    ----------------------------------------------
    int argc     : size of argv
    char** argv  : array of options
    File files[] : array containing the two files
   =============================================== */
void init_diff(int argc, char** argv, File files[]) {

    uint i = 0;

    // Parameters analyze
    #ifdef DEBUG
        START_TIMER;
        printf("Start of the options analysing...\n");
    #endif

    initialize_params();
	atexit(free_params_glob); // Add to the list
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

        // Paths
        files[i].path = diff_xmalloc(sizeof(char)*(diff_strlen(p->paths[i])+1));
        diff_strcpy(files[i].path, p->paths[i]);

        // Labels
        if(p->labels[i]) {
            files[i].label = diff_xmalloc(sizeof(char)*(diff_strlen(p->labels[i])+1));
            diff_strcpy(files[i].label, p->labels[i]);
        } else {
            files[i].label = files[i].path; // So we can always use the label
        }

        // Index
        files[i].i = NULL;
        files[i].f = NULL;
    }

    set_paths(files);
}


/* ===============================================
                     init_diff_r

    Initialize files with the arguments in argv.
    Used when called recursivly.
    ----------------------------------------------
    char* paths  : labels of the two files
    char* labels : paths of the two files
    File files[] : array containing the two files
   =============================================== */
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
