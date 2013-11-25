#include "paths.h"

void set_paths(File files[]) {

    int i = 0;
    char *tmp = NULL;

    #ifdef DEBUG
        printf("Start of paths analysing...\n");
    #endif

    for(; i < 2; i++) {
        if(stat(files[i].path, &files[i].st) != 0)
            send_error(files[i].path, NULL, NULL);
    }

    if(S_ISREG(files[0].st.st_mode) || S_ISFIFO(files[0].st.st_mode)) {
        if(S_ISREG(files[1].st.st_mode) || S_ISFIFO(files[1].st.st_mode)) {
            /* Fichier & fichier */
            files[0].type = files[1].type = T_FILE;

            #ifdef DEBUG
                printf("... two files detected\n--------------\n");
            #endif

        } else if(S_ISDIR(files[1].st.st_mode)) {
            /* Fichier & dossier */
            tmp = (char*)malloc(sizeof(char)*(strlen(files[0].path)+strlen(files[1].path)+2));
            sprintf(tmp, "%s/%s",files[1].path, files[0].path);
            free(files[1].path);
            files[1].path = tmp;

            #ifdef DEBUG
                printf("...file and directory detected\n");
            #endif

            if(stat(files[1].path, &files[1].st) != 0)
                exit_error(files[1].path, NULL, NULL);

            files[0].type = files[1].type = T_FILE;

            #ifdef DEBUG
                printf("...two files detected\n--------------\n");
            #endif

        } else
            exit(EXIT_ERROR);

    } else if(S_ISDIR(files[0].st.st_mode)) {
        if(S_ISREG(files[1].st.st_mode) || S_ISFIFO(files[1].st.st_mode)) {
            /* Dossier & fichier */

            tmp = (char*)malloc(sizeof(char)*(strlen(files[0].path)+strlen(files[1].path)+2));
            sprintf(tmp, "%s/%s",files[0].path, files[1].path);
            free(files[0].path);
            files[0].path = tmp;

            #ifdef DEBUG
                printf("...directory and file detected\n");
            #endif

            if(stat(files[0].path, &files[1].st) != 0)
                exit_error(files[0].path, NULL, NULL);

            files[0].type = files[1].type = T_FILE;

            #ifdef DEBUG
                printf("...two files detected\n--------------\n");
            #endif

        } else if(S_ISDIR(files[1].st.st_mode)) {
            /* Dossier & dossier */
            files[0].type = files[1].type = T_DIR;

            #ifdef DEBUG
                printf("..two directories detected\n--------------\n");
            #endif

        } else {
            exit_help();
            send_error(files[1].path, "not a file or a directory");
            return;
        }
    } else {
        exit_help();
        send_error(files[0].path, "not a file or a directory");
        return;
    }

}
