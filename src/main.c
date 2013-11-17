#include <stdio.h>
#include <stdlib.h>

#include "constant.h"
#include "init/init.h"

#include "diff/diff.h"

int main(int argc, char** argv){

    File files[2];

    #ifdef DEBUG
        printf("Debug version - Compiled at %s on %s\n--------------\n",__TIME__,__DATE__);
    #endif

    init_diff(argc, argv, files);

    if(files[0].type == T_DIR)
        printf("Dossier %s et %s\n", files[0].path, files[1].path);
    else if(files[0].type == T_FILE) {
        diff_file(files);
    }
    else
        puts("Autre");

    return 0;
}
