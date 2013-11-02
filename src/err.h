#ifndef H_ERR
#define H_ERR

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define CREATE_PARAMETERS

#include "constant.h"

/* Prototypes */
void show_help(void);
void send_error(const char *title, const char* msg);
void set_error(const char *title, const char* msg);
void diff_error(void);

#endif // H_ERR
