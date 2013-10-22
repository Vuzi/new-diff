#include "diff.h"

void sec_closedir(DIR *d) {

    if(closedir(d) == -1)
        perror("closedir()");
}

DIR *sec_opendir(const char* path) {

    DIR *d = opendir(path);

    if(!d)
        perror(path);

    return d;
}

int dir_search(const char* to_search, DIR* d) {

    int d_save = telldir(d), d_val = 0;
    struct dirent *dr = NULL;

    while ((dr = readdir(d)) != NULL) {
        if(strcmp(to_search, dr->d_name) == 0) {
            d_val = telldir(d);
            seekdir(d, d_save);
            return d_val;
        }
    }

    seekdir(d, d_save);
    return -1;
}

int diff_dir(const char* dir_name1, const char* dir_name2) {

    int ret = 0;

    DIR *d1 = sec_opendir(dir_name1), *d2 = sec_opendir(dir_name2);


    if(d1) {
        if(d2) {
            ret = diff_dir_make(dir_name1, d1, dir_name2, d2);
            sec_closedir(d1);
            sec_closedir(d2);
        } else {
            sec_closedir(d1);
            ret = 2;
        }

    } else
        ret = 2;

    return ret;
}

const char* get_type_char(struct stat *s) {

    if(S_ISREG(s->st_mode)) {
        if(s->st_size == 0)
            return "regular empty file";
        else
            return "regular file";
    } else if (S_ISDIR(s->st_mode))
        return "directory";
    else if (S_ISCHR(s->st_mode))
        return "character device";
    else if (S_ISBLK(s->st_mode))
        return "block device";
    else if (S_ISFIFO(s->st_mode))
        return "FIFO (named pipe)";
    #ifdef S_ISLNK
    else if (S_ISLNK(s->st_mode))
        return "symbolic link";
    #endif
    #ifdef S_ISSOCK
    else if (S_ISSOCK(s->st_mode))
        return "socket";
    #endif
    else
        return "unknown/other";

}

int diff_dir_make(const char* d1_name, DIR* d1, const char* d2_name, DIR* d2) {

    int ret = 0, tmp = 0;
    char *tmp_name1 = NULL, *tmp_name2 = NULL;

    struct dirent *dr1 = NULL, *dr2 = NULL;
    struct stat st1, st2;

    /* On passe le . & .. */
    seekdir(d1, 2);
    seekdir(d2, 2);

    /* Pour chaque dossier dans d1 */
    while ((dr1 = readdir(d1)) != NULL) {
        if((dr2 = readdir(d2)) != NULL) {
            /* S'il s'agit du même */
            if(strcmp(dr1->d_name, dr2->d_name) == 0) {

                tmp_name1 = (char*)malloc(sizeof(char)*(strlen(dr1->d_name)+strlen(d1_name)+2));
                sprintf(tmp_name1, "%s/%s", d1_name, dr1->d_name);

                tmp_name2 = (char*)malloc(sizeof(char)*(strlen(dr2->d_name)+strlen(d2_name)+2));
                sprintf(tmp_name2, "%s/%s", d2_name, dr2->d_name);

                /* On récupère le type */
                if(stat(tmp_name1, &st1) == 0) {
                    if(stat(tmp_name2, &st2) == 0) {
                        /* S'il sont du même type */
                        if(st1.st_mode == st2.st_mode) {
                            /* A voir/ faire
                                Si dossier ET -r ou si fichiers
                                1. Afficher la commande
                                2. Soit rappeller le main, ou relancer directement le programme */
                        }
                        /* Sinon ... */
                        else {
                            printf("File %s/%s is a %s while file %s/%s is a %s\n",
                                   d1_name, dr1->d_name, get_type_char(&st1), d2_name, dr2->d_name, get_type_char(&st2));
                        }
                    } else
                        perror(dr2->d_name);
                } else
                    perror(dr1->d_name);

                free(tmp_name1);
                free(tmp_name2);
            }
            /* Il ne s'agit pas du même */
            else {
                /* On retrouve d1 plus loin dans d2 */
                if((tmp = dir_search(dr1->d_name, d2)) != -1) {
                    /* Tout les elements de d2 jusqu'à tmp ne sont que dans d2 */
                    do {
                        printf("Only in %s: %s\n", d2_name, dr2->d_name);
                    } while((dr2 = readdir(d2)) != NULL && telldir(d2) < tmp);

                }
                /* On retrouve d2 plus loin dans d1 */
                else if((tmp = dir_search(dr2->d_name, d1)) != -1) {
                    /* Tout les elements de d1 jusqu'à tmp ne sont que dans d1 */
                    do {
                        printf("Only in %s: %s\n", d1_name, dr1->d_name);
                    } while((dr1 = readdir(d1)) != NULL && telldir(d1) < tmp);
                }
                else {
                    printf("Only in %s: %s\n", d1_name, dr1->d_name);
                    printf("Only in %s: %s\n", d2_name, dr2->d_name);
                }
            }
        } else {
            printf("Only in %s: %s\n", d1_name, dr1->d_name);
            ret = 1;
        }
    }

    return ret;
}
