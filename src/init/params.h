#ifndef _H_PARAMS
#define _H_PARAMS

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/regex.h"
#include "../my_types.h"

typedef enum _mode {
	NORMAL, IDENTICAL, DIFFERENT
} Mode;


typedef enum _output_style {
	NOT_SELECTED, REGULAR, CONTEXT, UNIFIED, COLUMNS, EDIT_SCRIPT, RCS, FORMAT
} output_style;

typedef struct _params {

	_bool brief;
	_bool report_identical_files;

	output_style o_style;
	int context;

	_bool generate_script;
	_bool generate_diff_file;

	uint show_max_char;
	_bool left_column; /* Ligne identique colonne de gauche */
	_bool suppress_common_lines; /* Ne pas afficher les lignes identiques */

	_bool show_c_function;
	char* show_function_line;
	char* label_1;
	char* label_2;

	_bool expand_tab;
	_bool align_tab;
	_bool size_tab;
	_bool delete_first_space;

	_bool out_relay;
	_bool recursive_dir;
	_bool new_file;
	_bool first_new_file;

	_bool ignore_case_filename;
	char* exclude_pattern;
	char* exclude_from;

	char* start_compare_file_in_dir;
	_bool start_compare_file;

	_bool ignore_case;
	_bool ignore_tab;
	_bool ignore_end_space;
	_bool ignore_change_space;
	_bool ignore_all_space;
	_bool ignore_blank_lines;
	regex_t *ignore_regex_match;

	_bool text;
	_bool binary;
	_bool strip_trailing_cr;

	char* group_format_GFMT;
	char* line_format_LFMT;
	char* line_type_format_LFMT;

	_bool minimal_diference;
	_bool horizontal_lines;
	_bool speed_large_files;

	_bool show_help;
	_bool show_version;

	char* pathLeft;
	char* pathRight;

	#ifdef DEBUG
	_bool d_show_index;
	_bool d_show_options;
	_bool d_show_diff;
	_bool d_interactive_mode;
	#endif

} Params;

#include "../constant.h"

/* == Prototypes == */
int make_param(char* option, char* argument);
void make_params(int argc, char **argv);
void initialize_params(void);
void free_params_glob(void);

#ifdef DEBUG
void print_params(Params* parameters);
#endif

#endif
