#ifndef _H_MY_TYPES
#define _H_MY_TYPES

#define uint unsigned int
#ifdef HANDLE_LARGE_FILES
    #define __USE_LARGEFILE64
    #define ulint unsigned long long int
    #define lint  long int
    #ifdef _WIN32
        #define SHOW_ulint "I64u"
        #define SHOW_lint "I64"
    #else
        #define SHOW_ulint "llu"
        #define SHOW_lint "ll"
    #endif
#else
    #define ulint unsigned long int
    #define lint long int
    #define SHOW_ulint "lu"
    #define SHOW_lint "l"
#endif
#define suint unsigned short int
#define sint  short int
#define _bool unsigned char
#define _true 1
#define _false 0

#define EXIT_ERROR 2
#define EXIT_DIFFERENTS_FILES 1
#define EXIT_IDENTICAL_FILES 0

#endif
