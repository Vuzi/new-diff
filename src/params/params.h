#ifndef PARAMS_H
#define PARAMS_H

#include <sys/types.h>
#include <regex2.h>

typedef enum _mode {
	NORMAL, IDENTICAL, DIFFERENT
} Mode;


typedef enum _output_style {
	NOT_SELECTED, REGULAR, CONTEXT, UNIFIED, COLUMNS, EDIT_SCRIPT, RCS, FORMAT
} output_style;

typedef struct _params {

	short brief;
	short report_identical_files;

	output_style o_style;
	short context;
	short unifier;
	short generate_script;
	short generate_diff_file;

	unsigned int show_max_char;
	short left_column; /* Ligne identique colonne de gauche */
	short suppress_common_lines; /* Ne pas afficher les lignes identiques */

	short show_c_function;
	char* show_function_line;
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
	regex_t *ignore_regex_match;

	short type_text;
	short remove_backspace_enter;
	char* show_diferent_fusion;

	char* group_format_GFMT;
	char* line_format_LFMT;
	char* line_type_format_LFMT;

	short minimal_diference;
	short horizontal_lines;
	short speed_large_files;

	short show_help;
	short show_version;

	char* pathLeft;
	char* pathRight;
} Params;


#include "../constant.h"

/* Prototypes */
void diff_init(int argc, char** argv);

void initialize_params(void);
int check_params(int argc, char **argv);
void free_params_glob(void);

char* append_char(char* src, char append);
void print_params(Params* parameters);

char toUpper(char c);
char toLower(char c);

#endif /* PARAMS_H */
