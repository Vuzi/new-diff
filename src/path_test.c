#include "path_test.h"

/* Prototypes statique */
static int get_type_path(const char* path);

static int get_type_path(const char* path) {
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

file_arg_case check_and_type_paths(const char* path1, const char* path2) {

    #ifdef DEBUG
        printf("Start of paths analysing...\n");
    #endif

    int val1 = get_type_path(path1), val2 = get_type_path(path2);

    if(val1 == 1) {
        if(val2 == 1)
            return DIR_DIR;
        else if (val2 == 2)
            return DIR_FILE;
    } else if(val1 == 2) {
        if(val2 == 1)
            return FILE_DIR;
        else if (val2 == 2)
            return FILE_FILE;
    }

    if(val1 == 0) {
        send_error(path1, "not a file or a directory", NULL);
        return NONE;
    }
    else if(val1 == -1) {
        perror(path1);
        return NONE;
    }

    if(val2 == 0) {
        send_error(path2, "not a file or a directory", NULL);
        return NONE;
    }
    else {
        perror(path1);
        return NONE;
    }

    #ifdef DEBUG
        printf("... paths analysing completed\n");
    #endif

}
