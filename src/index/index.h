#ifndef _H_INDEX
#define _H_INDEX

/* == Inlcudes == */
#include "../constant.h"
#include "hash.h"

/* == Structures/Enum == */

#define IS_EL_START(c) \
        (c == '\r' || c == '\n')

enum _END_LINE {
    LF /* \n */, CR /* \r */, CRLF /* \r\n */, END_OF_FILE /* EOF */
};

enum LINE_CHANGE_TYPE {
    LINE_NO_CHANGE = 0, LINE_IS_CHANGED = 1, LINE_ADD = 2, LINE_DEL = 3, LINE_CHANGE = 4
};

typedef enum _END_LINE END_LINE;

typedef struct _Line {

    ulint start;
    ulint lenght;
    END_LINE end_line;

    hash_t h;

    _bool is_c_func;

    enum LINE_CHANGE_TYPE modified;

} Line;

typedef struct _Index {

    Line* lines;
    ulint line_max;

} Index;

#include "../diff/diff.h"

/* == Prototypes == */
void index_file(File *file);
void index_free(File *file);

#ifdef DEBUG
void index_display(File *file);
#endif

#endif
