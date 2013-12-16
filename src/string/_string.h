#ifndef _H_DIFF_STRING
#define _H_DIFF_STRING

#include <sys/types.h>
#include <stdlib.h>
#include "../init/params.h"

/* == Prototypes == */
int diff_strcmp(const char *s1, const char *s2);
char diff_tolower(char c);
int diff_strcasecmp (const char *s1, const char *s2);
size_t diff_strlen (const char *s);
char* diff_strcpy(char *dest, const char *src);
char* diff_strchr (const char *s, int c);

void* diff_xcalloc(size_t n, size_t s);
void* diff_xmalloc(size_t n);

#endif // _H_DIFF_STRING
