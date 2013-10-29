#include "constant.h"
#include "params.h"

Params* initialize_params() {
	Params* return_params = NULL;

	return_params = (Params*) calloc(1, sizeof(Params));

    /* Certain en dur à des fins de test */
	return_params->brief = 0;
	return_params->report_identical_files = 0;

	return_params->o_style = NOT_SELECTED;
	return_params->context = 3;
	return_params->unifier = 3;

    return_params->show_max_char = 130;
    return_params->left_column = 0;
	return_params->suppress_common_lines = 0; /* Ne pas afficher les lignes identiques */

	return_params->show_c_function = 0;
	return_params->show_function_line = NULL;
	return_params->label = NULL;

	return_params->exclude_pattern = NULL;
	return_params->exclude_from = NULL;

	return_params->start_compare_file_in_dir = NULL;
	return_params->ignore_regex_match = NULL;
	return_params->show_diferent_fusion = NULL;

	return_params->group_format_GFMT = NULL;
	return_params->line_format_LFMT = NULL;
	return_params->line_type_format_LFMT = NULL;

	return_params->pathLeft = NULL;
	return_params->pathRight = NULL;

	return return_params;
}

int check_params(int argc, char **argv, Params *parameters) {
	int i = 0;
	int j = 0;
	int long_continue = 0;
	int result = 0;

	if (parameters != NULL) {

		if (argc < 3) {
			return 0;
		}

		for (i = 1; i < argc; i += 1) {
			printf("Reading a parameter : %s\n", argv[i]);

			if (result != 2) {
				result = read_param(argv[i], parameters, long_continue);

				if (result == 3) {
					j = 0;
					if (parameters->pathLeft != NULL) {
						parameters->pathRight = (char*) calloc(strlen(argv[i]) + 1, sizeof(char) );
						parameters->pathRight[strlen(argv[i])] = '\0';

						while (argv[i][j] != '\0') {
							parameters->pathRight[j] = argv[i][j];
							j += 1;
						}
					}
					else {
						parameters->pathLeft = (char*) calloc(strlen(argv[i]) + 1, sizeof(char) );
						parameters->pathLeft[strlen(argv[i])] = '\0';
						while (argv[i][j] != '\0') {
							parameters->pathLeft[j] = argv[i][j];
							j += 1;
						}
					}
				}
			} else {
				result = read_param(argv[i], parameters, long_continue);

				if (result == 3) {
					write_params(((argv[i - 1]) + 1), argv[i], parameters);
				}
				else {
					write_params(((argv[i - 1]) + 1), NULL, parameters);
				}
			}

			if (result == 1) {
				/* Last paramters "--PARAM[=VALUE] return 1" */
				long_continue = 1;
			} else if (result == -1) {
				return 0;
			}
		}

		return 1;
	} else {
		return 0;
	}
}

char toUpper(char c) {
	if (c >= 'a' && c <= 'z')
		return (c - ('a' - 'A'));

	return c;
}

char toLower(char c) {
	if (c >= 'A' && c <= 'Z')
		return (c + ('a' - 'A'));

	return c;
}

char* append_char(char* src, char append) {
	int i = 0;
	int s = 0;
	char* r = NULL;

	if (append == '\0') {
		return r;
	}

	if (src != NULL) {
		s = (strlen(src));
	} else {
		s = 0;
	}
	r = (char*) calloc(s + 2, sizeof(char) );

	if (r != NULL) {
		r[s + 1] = '\0';
		r[s] = append;

		if (src == NULL) {
			return r;
		}

		while (src[i] != '\0' && i < s) {
			r[i] = src[i];
			i += 1;
		}

		free(src);
		src = NULL;

	}

	return r;
}

int read_param(char *parameter, Params* parameters, int lc) {

	char test_dash = '-';
	char test_end = '\0';
	//char test_space = ' ';
	char test_equals = '=';
	char *pointer_char = (char*) malloc(sizeof(char) * 2);

	int count = 0;
	int size = strlen(parameter);

	unsigned short swap = 0;
	char *param = NULL;
	char* value = NULL;

	pointer_char[0] = '\0';
	pointer_char[1] = '\0';

	if (parameter[0] == test_dash && parameter[1] == test_dash) {
		if (size == 2) {
			if (parameters->pathLeft == NULL) {
				parameters->pathLeft = (char*) malloc(sizeof(char) * 2 );
				parameters->pathLeft[0] = '-';
				parameters->pathLeft[1] = '\0';
			}
			if (parameters->pathRight == NULL) {
				parameters->pathRight = (char*) malloc(sizeof(char) * 2);
				parameters->pathRight[0] = '-';
				parameters->pathRight[1] = '\0';
			}
			return 3;
		}

		if (lc == 1) {
			return 0;
		}

		count = 2;
		while (parameter[count] != test_end) {
			if (swap) {
				/* Add a letter for a value */
				value = append_char(value, parameter[count]);
			} else {
				if (parameter[count] == test_equals) {
					swap = 1;
				} else {
					/* Add a letter for a param */
					param = append_char(param, parameter[count]);
				}
			}
			count += 1;
		}

		write_params(param, value, parameters);
		return 1;
	} else if (parameter[0] == test_dash) {
		if (size == 1) {
			/* Entry read */

			if (parameters->pathLeft != NULL) {
				parameters->pathRight = (char*) malloc(sizeof(char) * 2);
				parameters->pathRight[0] = '-';
				parameters->pathRight[1] = '\0';
			}
			else {
				parameters->pathLeft = (char*) malloc(sizeof(char) * 2);
				parameters->pathLeft[0] = '-';
				parameters->pathLeft[1] = '\0';
			}
		}

		if (size < 3) {
			return 2;
		} else {
			count = 1;
			while (parameter[count] != '\0') {
				pointer_char[0] = parameter[count];
				write_params(pointer_char, NULL, parameters);
				count += 1;
			}
		}
	} else {
		return 3;
	}

	return 0;
}

int write_params(char* parameter, char* value, Params* parameters) {

	int i = 0;
	int val = 0;
	short use_value = -1;

	printf("-----\nParameter test : %s\n", parameter);

	if (value != NULL) {
		val = atoi(value);
		use_value = 0;
		printf("Value (text) : %s\n", value);
		printf("Parameter value : %i\n", val);
	}
	printf("-----\n");

	/* Writing a list of */
	if (!strcmp(parameter, "normal")) {
		parameters->brief = NORMAL;
	}
	else if (!strcmp(parameter, "brief") || !strcmp(parameter, "q")) {
		parameters->brief = DIFFERENT;
	}
	else if (!strcmp(parameter, "report-identical-files") || !strcmp(parameter, "s")) {
		parameters->brief = IDENTICAL;
	}
	else if (!strcmp(parameter, "context") || !strcmp(parameter, "c") || !strcmp(parameter, "C")) {
		if (value == NULL || val < 1 || !strcmp(parameter, "c")) {
			val = 3;
		}

		parameters->context = val;
		parameters->o_style = CONTEXT;
		use_value = 1;
	}
	else if (!strcmp(parameter, "unified") || !strcmp(parameter, "u") || !strcmp(parameter, "U")) {
		if (value == NULL || val < 1 || !strcmp(parameter, "u")) {
			val = 3;
		}

		parameters->unifier = val;
		parameters->o_style = UNIFIED;
		use_value = 1;
	}
	else if (!strcmp(parameter, "ed") || !strcmp(parameter, "e") || !strcmp(parameter, "C")) {
		parameters->generate_script = 1;
	}

	if (use_value == 0) {
		if (parameters->pathLeft != NULL) {

			if (parameters->pathRight != NULL) {
				return 0;
			}

			parameters->pathRight = (char*) calloc(strlen(value) + 1, sizeof(char) );
			parameters->pathRight[strlen(value)] = '\0';

			while (value[i] != '\0' && i < (int) (strlen(value) + 1)) {
				parameters->pathRight[i] = value[i];
				i += 1;
			}

		}
		else {
			parameters->pathLeft = (char*) calloc(strlen(value) + 1, sizeof(char) );
			parameters->pathLeft[strlen(value)] = '\0';

			while (value[i] != '\0' && i < (int) (strlen(value) + 1)) {
				parameters->pathLeft[i] = value[i];
				i += 1;
			}

		}
	}
	return 0;
}

void print_params(Params* parameters) {
	if (parameters == NULL) {
		return;
	}

	printf("-----\n");
	printf("Mode : %d\n", parameters->brief);

	printf("Context : %d\n", parameters->context);
	printf("Unifier : %d\n", parameters->unifier);
	printf("Generate script : %d\n", parameters->generate_script);
	printf("generate diff file : %d\n", parameters->generate_diff_file);

	//printf("Show colomns : %d\n", parameters->show_colomns);
	printf("Show max char : %d\n", parameters->show_max_char);
//	printf("Show left identical : %d\n", parameters->show_left_identical); /* Ligne identique colonne de gauche */

//	printf("Remove identical : %d\n", parameters->remove_identical);
	printf("Show C function : %d\n", parameters->show_c_function);
	printf("Show function line : %s\n", parameters->show_function_line); /* Montrer la ligne la plus récente égale à RE*/
	printf("Label : %s\n", parameters->label);

	printf("Expand tab : %d\n", parameters->expand_tab);
	printf("Align tab : %d\n", parameters->align_tab);
	printf("Size tab : %d\n", parameters->size_tab);
	printf("Delete first space : %d\n", parameters->delete_first_space);

	printf("Out relay : %d\n", parameters->out_relay);
	printf("Recursive dir : %d\n", parameters->recursive_dir);
	printf("New file : %d\n", parameters->new_file);
	printf("First New file : %d\n", parameters->first_new_file);

	printf("Ignore case filename : %d\n", parameters->ignore_case_filename);
	printf("Exclude pattern : %s\n", parameters->exclude_pattern);
	printf("Exclude from : %s\n", parameters->exclude_from);

	printf("Start compare file in dir : %d\n", parameters->start_compare_file_in_dir);
	printf("Start compare file : %s\n", parameters->start_compare_file);

	printf("Ignore case : %d\n", parameters->ignore_case);
	printf("Ignore tab : %d\n", parameters->ignore_tab);
	printf("Ignore end space : %d\n", parameters->ignore_end_space);
	printf("Ignore change space : %d\n", parameters->ignore_change_space);
	printf("Ignore all space : %d\n", parameters->ignore_all_space);
	printf("Ignore blank lines : %d\n", parameters->ignore_blank_lines);
	printf("Ignore regex match : %s\n", parameters->ignore_regex_match);

	printf("Type text : %d\n", parameters->type_text);
	printf("Remove backspace enter : %d\n", parameters->remove_backspace_enter);
	printf("Show different fusion : %s\n", parameters->show_diferent_fusion);

	printf("Group format GFMT : %s\n", parameters->group_format_GFMT);
	printf("Line format LFMT : %s\n", parameters->line_format_LFMT);
	printf("Line type format LFMT : %s\n", parameters->line_type_format_LFMT);

	printf("Minimal diference : %d\n", parameters->minimal_diference);
	printf("Horizontal lines : %d\n", parameters->horizontal_lines); /* Retire N lignes ayant des préfixes et suffixes identiques */
	printf("Speed large files : %d\n", parameters->speed_large_files);

	printf("Show help : %d\n", parameters->show_help);
	printf("Show version : %d\n", parameters->show_version);

	printf("Path left : %s\n", parameters->pathLeft);
	printf("Path right : %s\n", parameters->pathRight);
	printf("-----\n");
}
