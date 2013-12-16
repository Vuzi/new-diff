#include <stdio.h>
#include <stdlib.h>

#include "constant.h"
#include "init/init.h"

#include "diff/diff.h"

int main(int argc, char** argv){

    File files[2];
    int r = 0;

    #ifdef DEBUG
        printf("Debug version - Compiled at %s on %s\n--------------\n",__TIME__,__DATE__);
    #endif

    init_diff(argc, argv, files); // Initilise everything
    r = (files[0].type == T_DIR) ? diff_dir(files) : diff_file(files); // Analyse & display
    free_diff(files); // Free everything

    return r;
}
