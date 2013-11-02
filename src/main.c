#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define CREATE_PARAMETERS

#include "constant.h"

#include "path_test.h"

#include "diff_file/diff.h"
#include "diff_dir/diff.h"

#include <regex2.h>



// 0 = identique, 1 = différents, 2 = erreur
int main(int argc, char** argv){

    int ret = 0;
    char *tmp = NULL;

    diff_init(argc, argv);

    getchar();

    // File & File
    if((ret = check_and_type_paths((const char *)p->pathLeft, (const char *)p->pathRight)) == 0) {
        return diff_file((const char *)p->pathLeft, (const char *)p->pathRight);
    }
    // Dir & Dir
    else if (ret == 1) {
        return diff_dir((const char *)p->pathLeft, (const char *)p->pathRight);
    }
    // Dir & File
    else if (ret == 2) {
        tmp = (char*)malloc(sizeof(char)*(sizeof(p->pathLeft)+sizeof(p->pathRight)+2));
        sprintf(tmp, "%s/%s",p->pathLeft, p->pathRight);

        ret = diff_file(tmp, p->pathRight);

        free(tmp);
        return ret;
    }
    // File & Dir
    else if (ret == 3) {
        tmp = (char*)malloc(sizeof(char)*(sizeof(p->pathLeft)+sizeof(p->pathRight)+2));
        sprintf(tmp, "%s/%s",p->pathRight, p->pathLeft);

        ret = diff_file(p->pathLeft, tmp);

        free(tmp);
        return ret;
    }
    // Erreur
    else {
        return 2;
    }

}
