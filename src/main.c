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

    init_diff(argc, argv, files);

    if(files[0].type == T_DIR)
        r = diff_dir(files);
    else if(files[0].type == T_FILE)
        r = diff_file(files);
    else
        r = EXIT_ERROR;

    free_diff(files);

    return r;
}
