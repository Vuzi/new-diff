#include "_string.h"


/* ===============================================
                   diff_strcmp

    Compare s1 to s2. Return 0 when strings are
    identical, or the difference between the two
    first diffent characters.
    ----------------------------------------------
    const char* s1 : String to compare
    const char* s1 : String to compare
   =============================================== */
int diff_strcmp(const char *s1, const char *s2) {
    while(*s1 == *s2) {
        if(*s1 == '\0')
            return 0;
        s1++, s2++;
    }
    return *s1 - *s2;
}


/* ===============================================
                   diff_tolower

    Return a lower case version of c when possible
    (i.e. c must be between A and Z), otherwise
    c is returned.
    ----------------------------------------------
    char c : Char to be set to lower case
   =============================================== */
char diff_tolower(char c) {
    if(c >= 'A' && c <= 'Z')
        c += 32;
    return c;
}


/* ===============================================
                   diff_strcasecmp

    Compare s1 to s2 ignoring the case. Return 0
    when strings are identical, or the difference
    between the two first diffent characters.
    ----------------------------------------------
    const char* s1 : String to compare
    const char* s1 : String to compare
   =============================================== */
int diff_strcasecmp(const char *s1, const char *s2) {
    char s1_c = diff_tolower(*s1), s2_c = diff_tolower(*s2);

    while(s1_c == s2_c) {
        if(s1_c == '\0')
            return 0;
        s1++, s2++;
        s1_c = diff_tolower(*s1), s2_c = diff_tolower(*s2);
    }
    return s1_c - s2_c;
}


/* ===============================================
                   diff_strlen

    Return the length of the string s, excluding
    the terminating null byte.
    ----------------------------------------------
    const char* s1 : String to get the length
   =============================================== */
size_t diff_strlen(const char *s) {
    size_t size = 0;
    while(*s != '\0')
        size++, s++;
    return size;
}


/* ===============================================
                   diff_strcpy

    Copy the string src into the string dest
    including the null terminating byte. dest must
    be large enough to receive src.
    ----------------------------------------------
    char* dest      : Where to copy
    const char* src : What to copy
   =============================================== */
char* diff_strcpy(char *dest, const char *src) {

    while(*src != '\0') {
        *dest = *src;
        dest++, src++;
    }
    *dest = '\0';

    return dest;
}


/* ===============================================
                   diff_strchr

    Search the char c into the string s. The null
    terminating byte is ignored in the search.
    ----------------------------------------------
    const char* s : String where to search
    int c         : Charactere to search
   =============================================== */
char* diff_strchr(const char *s, int c) {
    while(*s != '\0') {
        if((int)*s == c)
            return (char*)s;
        s++;
    }
    return NULL;
}


/* ===============================================
                   diff_xcalloc

    Try to calloc n block(s) of s byte(s), and
    return the result if the calloc is sucessful
    or print the error and exit the program.
    ----------------------------------------------
    size_t n : Number of block to allocate
    size_t s : Number of bytes to allocate
   =============================================== */
void* diff_xcalloc(size_t n, size_t s) {
    void* ptr = calloc(n, s);

    if(!ptr)
        exit_error(NULL, NULL);

    return ptr;
}


/* ===============================================
                   diff_xmalloc

    Try to malloc s byte(s), and return the result
    if the calloc is sucessful or print the error
    and exit the program.
    ----------------------------------------------
    size_t s : Number of bytes to allocate
   =============================================== */
void* diff_xmalloc(size_t s) {
    void* ptr = malloc(s);

    if(!ptr)
        exit_error(NULL, NULL);

    return ptr;
}
