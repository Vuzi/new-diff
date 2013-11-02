#ifndef PATH_TEST_H_INCLUDED
#define PATH_TEST_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define CREATE_PARAMETERS

#include "constant.h"

enum e_file_arg_case {
    NONE = -1,
    FILE_FILE = 0,
    DIR_DIR = 1,
    DIR_FILE = 2,
    FILE_DIR = 3
};

typedef enum e_file_arg_case file_arg_case;

/* Prototypes */
file_arg_case check_and_type_paths(const char* path1, const char* path2);

#endif // PATH_TEST_H_INCLUDED
