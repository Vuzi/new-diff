/* == Inlcudes == */

#include <dirent.h>
#include "../string/_string.h"
#include "../constant.h"
#include "../index/index.h"
#include "snake.h"
#include "../print/print.h"

#ifndef _H_DIFF_S
#define _H_DIFF_S

/* == Structures/Enum == */
typedef enum _File_type {
    T_DIR, T_FILE, T_STDIN, T_NONE
}File_type;

typedef struct _File{

	FILE *f;
	DIR *d;

	Index *i;

	char *path;
	char *label;

	struct stat st;
	File_type type;
	_bool empty;

}File;

#endif

#ifndef _H_DIFF
#define _H_DIFF
#include "../init/init.h"

/* == Prototypes == */
FILE* sec_fopen(const char* path, const char* mode);
void sec_fclose(FILE *f);

void sec_closedir(DIR *d);
DIR *sec_opendir(const char* path);

int diff_file(File files[]);
int diff_dir(File files[]);

void free_diff(File files[]);
void free_diff_r(File files[]);
ulint diff_get_length(Index* index, ulint i);

#endif
