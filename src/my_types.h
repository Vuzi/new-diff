#ifndef _H_MY_TYPES
#define _H_MY_TYPES

#ifdef HANDLE_LARGE_FILES
    #define __USE_LARGEFILE64
    typedef unsigned long long int ulint;
    typedef long long int lint;
    #ifdef _WIN32
        #define SHOW_ulint "I64u"
        #define SHOW_lint "I64"
    #else
        #define SHOW_ulint "llu"
        #define SHOW_lint "ll"
    #endif
#else
    typedef unsigned long int ulint;
    typedef long int lint;
    #define SHOW_ulint "lu"
    #define SHOW_lint "l"
#endif

typedef unsigned int uint;
typedef unsigned short int suint;
typedef short int sint;
typedef unsigned char _bool;
#define _true 1
#define _false 0

#define EXIT_ERROR 2
#define EXIT_DIFFERENTS_FILES 1
#define EXIT_IDENTICAL_FILES 0

#endif
