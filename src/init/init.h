#ifndef _H_DIFF_INIT
#define _H_DIFF_INIT

#include "../constant.h"
#include "params.h"
#include "paths.h"

void init_diff(int argc, char** argv, File files[]);
void init_diff_r(char* paths[], char* labels[], File files[]);

#endif // _H_DIFF_INIT
