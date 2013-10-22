#ifndef PARAMS_H
#define PARAMS_H

#include "constant.h"

typedef enum _mode {
	NORMAL, IDENTICAL, DIFFERENT
} Mode;

typedef struct _params {
	Mode brief;

	short context;
	short unifier;
	short generate_script;
	short generate_diff_file;

	short show_colomns;
	short show_max_char;
	short show_left_identical; /* Ligne identique colonne de gauche */

	short remove_identical;
	short show_c_function;
	char* show_function_line; /* Montrer la ligne la plus récente égale à RE*/
	char* label;

	short expand_tab;
	short align_tab;
	short size_tab;
	short delete_first_space;

	short out_relay;
	short recursive_dir;
	short new_file;
	short first_new_file;

	short ignore_case_filename;
	char* exclude_pattern;
	char* exclude_from;

	char* start_compare_file_in_dir;
	short start_compare_file;

	short ignore_case;
	short ignore_tab;
	short ignore_end_space;
	short ignore_change_space;
	short ignore_all_space;
	short ignore_blank_lines;
	char* ignore_regex_match;

	short type_text;
	short remove_backspace_enter;
	char* show_diferent_fusion;
	
	char* group_format_GFMT;
	char* line_format_LFMT;
	char* line_type_format_LFMT;

	short minimal_diference;
	short horizontal_lines; /* Retire N lignes ayant des préfixes et suffixes identiques */
	short speed_large_files;

	short show_help;
	short show_version;

	char* pathLeft;
	char* pathRight;
} Params;

Params* initialize_params();
int check_params(int argc, char **argv, Params *parameters);
int read_param(char *parameter, Params* parameters);

char toUpper(char c);
char toLower(char c);

#endif /* PARAMS_H */