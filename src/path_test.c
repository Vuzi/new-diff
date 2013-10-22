#include "path_test.h"

int get_type_path(const char* path) {
    struct stat st;

    if(stat(path, &st) == 0) {
        /* Si dossier */
        if(S_ISDIR(st.st_mode))
            return 1;
        /* Si fichier (ou fifo) */
        else if (S_ISREG(st.st_mode) || S_ISFIFO(st.st_mode))
            return 2;
        else
            return 0;
    } else
        return -1;

}

/* On cherche à savoir de quel cas de figure il s'agit
   Cas possibles :
   0:FILE FILE
   1:DIR DIR
   2:DIR FILE
   3:FILE DIR */
int check_and_type_paths(const char** paths) {

    int val1 = get_type_path(paths[0]), val2 = get_type_path(paths[1]);

    if(val1 == 1) {
        if(val2 == 1)
            return 1;
        else if (val2 == 2)
            return 2;
    } else if(val1 == 2) {
        if(val2 == 1)
            return 3;
        else if (val2 == 2)
            return 0;
    }

    if(val1 == 0) {
        printf("%s : not a file or a directory", paths[0]);
        return -1;
    }
    else if(val1 == -1) {
        perror(paths[0]);
        return -1;
    }

    if(val2 == 0) {
        printf("%s : not a file or a directory", paths[1]);
        return -1;
    }
    else {
        perror(paths[1]);
        return -1;
    }

}
