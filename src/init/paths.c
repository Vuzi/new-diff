#include "paths.h"

void set_paths(File files[]) {

    int i = 0;
    char *tmp = NULL;
    _bool files_tests[2] = { _true, _true};

    #ifdef DEBUG
        printf("Start of paths analysing...\n");
    #endif

    // Get the stats
    for(; i < 2; i++) {
        if(stat(files[i].path, &files[i].st) != 0) {
            files_tests[i] = _false;
            if(!(p->new_file && errno == ENOENT))
                send_error(files[i].path, NULL, NULL);
        }
    }

    // If possible
    if((files_tests[0] && files_tests[1]) ||
      ((files_tests[0] || files_tests[1]) && p->new_file)) {

        // Get their type
        for( i = 0; i < 2; i++) {
            if(files_tests[i]) {
                if(S_ISREG(files[i].st.st_mode) || S_ISFIFO(files[i].st.st_mode)) {
                    files[i].type = T_FILE;
                    if(files[i].st.st_size == 0)
                        files[i].empty = _true;
                } else if(S_ISDIR(files[i].st.st_mode)) {
                    files[i].type = T_DIR;
                } else {
                    exit_help();
                    exit_error(files[i].path, "not a file or a directory");
                }
            } else
                files[i].type = T_NONE;
        }

        // Not the same
        if(files[0].type != files[1].type) {
            // Nothing and something
            if(files[0].type == T_NONE) {
                files[0].type = files[1].type;
                files[0].empty  =_true;
            } else if(files[1].type == T_NONE) {
                files[1].type = files[0].type;
                files[1].empty  =_true;
            }
            else {
                // File & dir
                if(files[0].type == T_FILE) {
                    tmp = (char*)malloc(sizeof(char)*(diff_strlen(files[0].path)+diff_strlen(files[1].path)+2));
                    sprintf(tmp, "%s/%s",files[1].path, files[0].path);
                    free(files[1].path);
                    files[1].path = tmp;

                    if(stat(files[1].path, &files[1].st) != 0)
                        exit_error(files[1].path, NULL, NULL);

                    if(!(S_ISREG(files[1].st.st_mode) || S_ISFIFO(files[1].st.st_mode))) {
                        exit_help();
                        exit_error(files[1].path, "not a regular file");
                    }

                    files[1].type = T_FILE;
                }
                // Dir and file
                else {
                    tmp = (char*)malloc(sizeof(char)*(diff_strlen(files[0].path)+diff_strlen(files[1].path)+2));
                    sprintf(tmp, "%s/%s",files[0].path, files[1].path);
                    free(files[0].path);
                    files[0].path = tmp;

                    if(stat(files[0].path, &files[1].st) != 0)
                        exit_error(files[0].path, NULL, NULL);

                    if(!(S_ISREG(files[0].st.st_mode) || S_ISFIFO(files[0].st.st_mode))) {
                        exit_help();
                        exit_error(files[0].path, "not a regular file");
                    }

                    files[0].type = T_FILE;
                }
            }
        }

        #ifdef DEBUG
            if(files[0].type == T_FILE)
                printf("...two files detected\n--------------\n");
            else
                printf("..two directories detected\n--------------\n");
        #endif
      }
      // Error
      else {
        if(!files_tests[0] && !files_tests[1] && p->new_file) // Erreur spéciale à afficher
            send_error(NULL, "both files are non-existent");
        exit(EXIT_ERROR);
      }
}
