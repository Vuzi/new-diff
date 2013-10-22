#ifndef PATH_TEST_H_INCLUDED
#define PATH_TEST_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

/* Prototypes */
int get_type_path(const char* path);
int check_and_type_paths(const char** paths);

#endif // PATH_TEST_H_INCLUDED
