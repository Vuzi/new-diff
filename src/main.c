#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define CREATE_PARAMETERS

#include "params/constant.h"

#include "path_test.h"

#include "diff_file/diff.h"
#include "diff_dir/diff.h"


// 0 = identique, 1 = différents, 2 = erreur
int main(int argc, char** argv){

    int ret = 0;
    char *tmp = NULL;

    p = initialize_params();

    if(argc < 3) {
        printf("Pas assez d'arguments");
        return 2;
    }

    // File & File
    if((ret = check_and_type_paths((const char **)(argv+1))) == 0) {
        return diff_file((const char *)(*(argv+1)), (const char *)(*(argv+2)));
    }
    // Dir & Dir
    else if (ret == 1) {
        return diff_dir((const char *)(*(argv+1)), (const char *)(*(argv+2)));
    }
    // Dir & File
    else if (ret == 2) {
        tmp = (char*)malloc(sizeof(char)*(sizeof(*(argv+1))+sizeof(*(argv+2))+2));
        sprintf(tmp, "%s/%s",*(argv+1), *(argv+2));

        ret = diff_file(tmp, *(argv+2));

        free(tmp);
        return ret;
    }
    // File & Dir
    else if (ret == 3) {
        tmp = (char*)malloc(sizeof(char)*(sizeof(*(argv+1))+sizeof(*(argv+2))+2));
        sprintf(tmp, "%s/%s",*(argv+2), *(argv+1));

        ret = diff_file(*(argv+1), tmp);

        free(tmp);
        return ret;
    }
    // Erreur
    else {
        return 2;
    }

}
