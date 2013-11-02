
#include "params.h"

static int read_param(char *parameter, Params* parameters, int lc);
static int write_params(char* params, char* value, Params *parameter);

void diff_init(int argc, char** argv) {

	atexit(free_params_glob); // Ajout à la liste
    atexit(diff_error);

    initialize_params();
    check_params(argc, argv);

    /* A mettre plus tard au bon endroit */
    if(p->o_style == NOT_SELECTED) {
        if(p->show_c_function)
            p->o_style = CONTEXT;
        else
            p->o_style = REGULAR;

    }
}


void initialize_params() {
	p = (Params*) calloc(1, sizeof(Params));

	/* Certain en dur à des fins de test */
	p->brief = 0;
	p->report_identical_files = 0;

	p->o_style = NOT_SELECTED;
	p->context = 3;
	p->unifier = 3;

	p->show_max_char = 130;
	p->left_column = 0;
	p->suppress_common_lines = 0; /* Ne pas afficher les lignes identiques */

	p->show_c_function = 0;
	p->show_function_line = NULL;
	p->label = NULL;

	p->exclude_pattern = NULL;
	p->exclude_from = NULL;

	p->start_compare_file_in_dir = NULL;
	p->ignore_regex_match = NULL;
	p->show_diferent_fusion = NULL;

	p->group_format_GFMT = NULL;
	p->line_format_LFMT = NULL;
	p->line_type_format_LFMT = NULL;

	p->pathLeft = NULL;
	p->pathRight = NULL;

}

int check_params(int argc, char **argv) {
	int i = 0;
	int j = 0;
	int long_continue = 0;
	int result = 0;

	char param_error[512] = {0};

	if (p != NULL) {

		for (i = 1; i < argc; i += 1) {
			printf("Reading a parameter : %s\n", argv[i]);

			if (result != 2) {
				result = read_param(argv[i], p, long_continue);

				if (result == 3) {
					j = 0;
					if (p->pathLeft != NULL) {
						p->pathRight = (char*) calloc(strlen(argv[i]) + 1, sizeof(char) );
						p->pathRight[strlen(argv[i])] = '\0';

						while (argv[i][j] != '\0') {
							p->pathRight[j] = argv[i][j];
							j += 1;
						}
					}
					else if (result == 4) {
						if ((i + 1) < argc) {
							write_params(argv[i], argv[(i + 1)], p);
						}
					}
					else {
						p->pathLeft = (char*) calloc(strlen(argv[i]) + 1, sizeof(char) );
						p->pathLeft[strlen(argv[i])] = '\0';
						while (argv[i][j] != '\0') {
							p->pathLeft[j] = argv[i][j];
							j += 1;
						}
					}
				}
			}
			else {
				result = read_param(argv[i], p, long_continue);

				if (result == 3) {
					write_params(((argv[i - 1]) + 1), argv[i], p);
				}
				else {
					write_params(((argv[i - 1]) + 1), NULL, p);
				}
			}

			if (result == 1) {
				/* Last paramters "--PARAM[=VALUE] return 1" */
				long_continue = 1;
			}
			else if (result == -1) {
				return 0;
			}
		}

		if(i < 3 ) { // Si <3, il manque des opérateurs
            show_help();
            sprintf(param_error, "missing operand after '%s'", argv[i - 1]); // Fonctionne car >= 1
            send_error(NULL, param_error);
		}

		return 1;
	}
	else {
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
	}
	else {
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

static int read_param(char *parameter, Params* parameters, int lc) {

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
				parameters->pathLeft = (char*) malloc(sizeof(char) * 2);
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
			}
			else {
				if (parameter[count] == test_equals) {
					swap = 1;
				}
				else {
					/* Add a letter for a param */
					param = append_char(param, parameter[count]);
				}
			}
			count += 1;
		}

		if (write_params(param, value, parameters) == 4) {
			return 4;
		}
		return 1;
	}
	else if (parameter[0] == test_dash) {
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
		}
		else {
			count = 1;
			while (parameter[count] != '\0') {
				pointer_char[0] = parameter[count];
				write_params(pointer_char, NULL, parameters);
				count += 1;
			}
		}
	}
	else {
		return 3;
	}

	return 0;
}

static int write_params(char* parameter, char* value, Params* parameters) {

	int i = 0;
	int val = 0;
	short use_value = -1;

	char param_error[512] = {0};

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
		parameters->o_style = REGULAR;
	}
	else if (!strcmp(parameter, "brief") || !strcmp(parameter, "q")) {
		parameters->brief = 1;
	}
	else if (!strcmp(parameter, "report-identical-files") || !strcmp(parameter, "s")) {
		parameters->report_identical_files = 1;
	}
	else if (!strcmp(parameter, "context") || !strcmp(parameter, "c") || !strcmp(parameter, "C")) {
		if (value == NULL || val < 0 || !strcmp(parameter, "c")) {
			val = 3;
		}

		parameters->context = val;
		parameters->o_style = CONTEXT;
		use_value = 1;
	}
	else if (!strcmp(parameter, "unified") || !strcmp(parameter, "u") || !strcmp(parameter, "U")) {
		if (value == NULL || val < 0 || !strcmp(parameter, "u")) {
			val = 3;
		}

		parameters->unifier = val;
		parameters->o_style = UNIFIED;
		use_value = 1;
	}
	else if (!strcmp(parameter, "ed") || !strcmp(parameter, "e") || !strcmp(parameter, "C")) {
		parameters->generate_script = 1;
	}
	else if (!strcmp(parameter, "rcs") || !strcmp(parameter, "n")) {
		parameters->generate_diff_file = 1;
	}
	else if (!strcmp(parameter, "side-by-side") || !strcmp(parameter, "y")) {
		parameters->o_style = COLUMNS;
	}
	else if (!strcmp(parameter, "width") || !strcmp(parameter, "W")) {
		if (value == NULL || val < 1 || !strcmp(parameter, "W")) {
			val = 130;
		}

		parameters->show_max_char = val;
		use_value = 1;
	}
	else if (!strcmp(parameter, "left-column")) {
		parameters->o_style = COLUMNS;
		parameters->left_column = 1;
	}
	else if (!strcmp(parameter, "suppress-common-lines")) {
		parameters->o_style = COLUMNS;
		parameters->suppress_common_lines = 1;
	}
	else if (!strcmp(parameter, "show-c-function") || !strcmp(parameter, "p")) {
		parameters->show_c_function = 1;
	}
	else if (!strcmp(parameter, "label")) {
		parameters->label = NULL;
		return 4; // Special return
	}
	else if (!strcmp(parameter, "ignore-case") || !strcmp(parameter, "i")) {
		parameters->ignore_case = 1;
	}
	else if (!strcmp(parameter, "ignore-tab-expansion") || !strcmp(parameter, "E")) {
		parameters->ignore_tab = 1;
	}
	else if (!strcmp(parameter, "ignore-trailing-space") || !strcmp(parameter, "Z")) {
		parameters->ignore_end_space = 1;
	}
	else if (!strcmp(parameter, "ignore-space-change") || !strcmp(parameter, "b")) {
		parameters->ignore_change_space = 1;
	}
	else if (!strcmp(parameter, "ignore-all-space") || !strcmp(parameter, "w")) {
		parameters->ignore_all_space = 1;
	}
	else if (!strcmp(parameter, "ignore-blank-lines") || !strcmp(parameter, "B")) {
		parameters->ignore_blank_lines = 1;
	}
	else if (!strcmp(parameter, "ignore-matching-lines") || !strcmp(parameter, "I")) {

		parameters->ignore_regex_match = (regex_t*)malloc(sizeof(regex_t));
		if(regcomp(parameters->ignore_regex_match, value, REG_NOSUB | REG_EXTENDED) == 0) {
            use_value = 1;
		}else {
            fputs("diff: error when compiling the regex expression\n", stderr);
            exit(2);
		}
	} else {
        show_help();
        sprintf(param_error, "invalid option -- '%s'", parameter);
        send_error(NULL, param_error);
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
	printf("Show function line : %s\n", parameters->show_function_line);
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

	printf("Start compare file in dir : %s\n", parameters->start_compare_file_in_dir);
	printf("Start compare file : %d\n", parameters->start_compare_file);

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

void free_params_glob(void) {

    if(p) {
        /* Ici libérer tout les char* et cie */

        if(p->ignore_regex_match)
            regfree(p->ignore_regex_match);

        free(p);
        p = NULL;
    }
}
