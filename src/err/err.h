#ifndef _H_ERR
#define _H_ERR

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../string/_string.h"
#include <stdarg.h>

#include "../constant.h"

/* Defines */
void exit_error(const char* title, const char* msg, ...);
void send_error(const char* title, const char* msg, ...);
void exit_help(void);

#endif
