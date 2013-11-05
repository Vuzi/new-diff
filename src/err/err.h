#ifndef H_ERR
#define H_ERR

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdarg.h>

#define CREATE_PARAMETERS

#include "../constant.h"

/* Defines */
void exit_error(const char* title, const char* msg, ...);
void send_error(const char* title, const char* msg, ...);
void exit_help(void);

#endif // H_ERR
