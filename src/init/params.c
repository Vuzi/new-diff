#include "params.h"
#include "../print/print.h" // Fonction print

Params *p = NULL;
suint diff_stderr_show_help = 0;

static void set_file_name(char *name);
static void set_output_style(output_style style);
static void set_context(char* val);
static void set_width(char* val);

void initialize_params(void) {
	p = (Params*) calloc(1, sizeof(Params));

	p->o_style = NOT_SELECTED;
	p->context = 3;

	p->width = 130;
	p->size_tab = 8;

	p->show_regex_function = NULL;
	p->label_1 = NULL;
	p->label_2 = NULL;

	p->exclude_pattern = NULL;
	p->exclude_from = NULL;

	p->start_compare_file_in_dir = NULL;

	p->ignore_blank_lines = NULL;
	p->ignore_regex_match = NULL;

	p->group_format_GFMT = NULL;
	p->line_format_LFMT = NULL;
	p->line_type_format_LFMT = NULL;

	p->pathLeft = NULL;
	p->pathRight = NULL;

}


void make_params(int argc, char **argv) {

    int i = 1, j = 0, arg_length;
    short int no_more_op = 0;

    char short_tmp[2] = {0};
    char *arg_tmp = NULL;

    /* Pour chacun des arguments */
    for (i = 1; i < argc; i++) {

        arg_length = strlen(argv[i]);

        /* Si on commence par un tiret */
        if(!no_more_op && arg_length > 0 && argv[i][0] == '-') {
            /* Si on a un second tiret, argument long */
            if(arg_length > 1 && argv[i][1] == '-') {
                /* Si ce double tiret est tout seul, les options sont terminées */
                if(arg_length == 2) {
                    no_more_op = 1;
                }
                /* Argument long */
                else {

                    if(argv[i][2] == '=') {
                        exit_help();
                        exit_error(NULL, "option '%s' is ambigous", argv[i]);
                    }

                    arg_tmp = strchr(argv[i]+2, (int)'=');

                    if(arg_tmp) {
                        arg_tmp = '\0';
                        arg_tmp++; // Pour couper en deux une même chaine
                        make_param(argv[i]+2, arg_tmp);
                    } else {
                        if(argc < i+1)
                            i += make_param(argv[i]+2, argv[i+1]);
                        else
                            make_param(argv[i]+2, NULL);
                    }
                }
            }
            /* Tiret seul : stdin comme nom de fichier */
            else if(arg_length == 1) {
                set_file_name(argv[i]);
            }
            /* Argument(s) court(s) */
            else {
                /* Plusieurs : valeurs ignorées */
                if(arg_length > 2) {
                    /* On peut en mettre plusieurs à la suite */
                    for(j = 1; j < arg_length; j++) {
                        short_tmp[0] = argv[i][j];

                        if(j+1 < arg_length) {
                            if(make_param(short_tmp, argv[i]+j+1)) // argument + suite ligne
                                j = arg_length-1;
                        } else if(i+1 < argc)
                            i += make_param(short_tmp, argv[i+1]); // argument + suivant
                        else
                            make_param(short_tmp, NULL);
                    }
                }
                /* Un seul : On peut avoir des valeurs à la suite */
                else {
                    if(i+1 < argc)
                        i += make_param(argv[i]+1, argv[i+1]);
                    else
                        make_param(argv[i]+1, NULL);
                }
            }
        }
        /* Pas de tiret, c'est un nom de fichier */
        else {
            set_file_name(argv[i]);
        }

    }

    /* Si aucune sortie, celle par défaut */
    if(p->o_style == NOT_SELECTED) {
        if(p->show_regex_function)
            p->o_style = CONTEXT;
        else
            p->o_style = REGULAR;
    }

    /* Si il manque un path, erreur */
    if(!(p->pathLeft)) {
        exit_help();
        exit_error(NULL, "missing operand after 'diff'", NULL);
    } else if (!(p->pathRight)) {
        exit_help();
        exit_error(NULL, "missing operand after '%s'", p->pathLeft);
    }
}


static void set_file_name(char *name) {

    #ifdef DEBUG
        printf("Filename : %s\n--------\n", name);
    #endif

    if(p) {
        if(!(p->pathLeft)) {
            p->pathLeft = name;
        } else if(!(p->pathRight)) {
            p->pathRight = name;
        } else {
            exit_help();
            exit_error(NULL, "extra operand '%s'", name);
        }
    }
}


static void set_output_style(output_style style) {
    if(p->o_style != NOT_SELECTED && style != p->o_style) {
        exit_help();
        exit_error(NULL, "conflicting output style options", NULL);
    } else
        p->o_style = style;
}


static void set_context(char* val) {

    long int n = 0;
    char* str;

    n = strtol(val, &str, 10);

    if(val[0] != '\0' && *str == '\0') {
        if(n >= 0)
            p->context = (int)n;
    } else {
        exit_help();
        exit_error(NULL, "invalid context length '%s'", val);
    }
}

static void set_width(char* val) {

    long int n = 0;
    char* str;

    n = strtol(val, &str, 10);

    if(val[0] != '\0' && *str == '\0') {
        if(n >= 0)
            p->width = (int)n;
        else if (n == 0) {
            exit_help();
            exit_error(NULL, "invalid width '%s'", val);
        }
    } else {
        exit_help();
        exit_error(NULL, "invalid width '%s'", val);
    }
}

int make_param(char* option, char* argument) {

    #ifdef DEBUG
        printf("Option : %s\nArgument : %s\n--------\n", option, argument);
    #endif

    if(p) {
        if (!strcmp(option, "normal")) {
            set_output_style(REGULAR);
            return 0;
        }
        else if (!strcmp(option, "brief") || !strcmp(option, "q")) {
            p->brief = _true;
            return 0;
        }
        else if (!strcmp(option, "report-identical-files") || !strcmp(option, "s")) {
            p->report_identical_files = _true;
            return 0;
        }
        else if (!strcmp(option, "c")) {
            set_output_style(CONTEXT);
            return 0;
        }
        else if (!strcmp(option, "C")) {

            if(!argument) {
                exit_help();
                exit_error(NULL, "option requires an argument -- '%s'", option);
            }

            set_output_style(CONTEXT);
            set_context(argument);
            return 1;

        }
        else if (!strcmp(option, "context")) {

            set_output_style(CONTEXT);

            if(argument) {
                set_context(argument);
                return 1;
            }

            return 0;
        }
        else if (!strcmp(option, "u")) {
            set_output_style(UNIFIED);
        }
        else if (!strcmp(option, "U")) {

            if(!argument) {
                exit_help();
                exit_error(NULL, "option requires an argument -- '%s'", option);
            }

            set_output_style(UNIFIED);
            set_context(argument);
            return 1;

        }
        else if (!strcmp(option, "unified")) {

            set_output_style(UNIFIED);

            if(argument) {
                set_context(argument);
                return 1;
            }

            return 0;
        }
        else if (!strcmp(option, "ed") || !strcmp(option, "e")) {
            set_output_style(EDIT_SCRIPT);
            return 0;
        }
        else if (!strcmp(option, "rcs") || !strcmp(option, "n")) {
            set_output_style(RCS);
            return 0;
        }
        else if (!strcmp(option, "side-by-side") || !strcmp(option, "y")) {
            set_output_style(COLUMNS);
            return 0;
        }
        else if (!strcmp(option, "width") || !strcmp(option, "W")) {

            if(!argument) {
                exit_help();
                exit_error(NULL, "option requires an argument -- '%s'", option);
            }

            set_width(argument);
            return 1;
        }
        else if (!strcmp(option, "left-column")) {
            p->left_column = _true;
            return 0;
        }
        else if (!strcmp(option, "suppress-common-lines")) {
            p->suppress_common_lines = _true;
            return 0;
        }
        else if (!strcmp(option, "show-c-function") || !strcmp(option, "p")) {

            if (p->show_regex_function) {
                exit_help();
                exit_error(NULL, "conflicting type of matching function to show", option);
            }

            p->show_regex_function = (regex_t*)malloc(sizeof(regex_t)); // regex

            if(regcomp(p->show_regex_function, C_FUNCTION_REGEX, REG_NOSUB | REG_EXTENDED) != 0) { // Compilation
                exit_help();
                exit_error(NULL, "regex '%s' of c function is invalid (and shouldn't).", C_FUNCTION_REGEX);
            }

            return 0;
        }
        else if (!strcmp(option, "show-function-line") || !strcmp(option, "F")) {

            if(!argument) {
                exit_help();
                exit_error(NULL, "option '%s' requires an argument", option);
            } else if (p->show_regex_function) {
                exit_help();
                exit_error(NULL, "conflicting type of matching function to show", option);
            }

            p->show_regex_function = (regex_t*)malloc(sizeof(regex_t)); // regex

            if(regcomp(p->show_regex_function, argument, REG_NOSUB | REG_EXTENDED) != 0) { // Compilation
                exit_help();
                exit_error(NULL, "regex '%s' is invalid", argument);
            }

            return 1;
        }
        else if (!strcmp(option, "label") || !strcmp(option, "L") ) {

            if(!argument) {
                exit_help();
                exit_error(NULL, "option '%s' requires an argument", option);
            }

            if(!p->label_1)
                p->label_1 = argument;
            else if(!p->label_2)
                p->label_2 = argument;
            else
                exit_error(NULL, "more than two labels given '%s'", option);
            return 1;
        }
        else if (!strcmp(option, "suppress-blank-empty")) {
            p->suppress_blank_empty = _true;
            return 0;
        }
        else if (!strcmp(option, "ignore-case") || !strcmp(option, "i")) {
            p->ignore_case = _true;
            return 0;
        }
        else if (!strcmp(option, "ignore-tab-expansion") || !strcmp(option, "E")) {
            p->ignore_tab_change = _true;
            return 0;
        }
        else if (!strcmp(option, "ignore-trailing-space") || !strcmp(option, "Z")) {
            p->ignore_end_space = _true;
            return 0;
        }
        else if (!strcmp(option, "ignore-space-change") || !strcmp(option, "b")) {
            p->ignore_space_change = _true;
            return 0;
        }
        else if (!strcmp(option, "ignore-all-space") || !strcmp(option, "w")) {
            p->ignore_all_space = _true;
            return 0;
        }
        else if (!strcmp(option, "ignore-blank-lines") || !strcmp(option, "B")) {

            p->ignore_blank_lines = (regex_t*)malloc(sizeof(regex_t)); // regex

            if(regcomp(p->ignore_blank_lines, BLANK_LINE_REGEX, REG_NOSUB | REG_EXTENDED) != 0) { // Compilation
                exit_help();
                exit_error(NULL, "regex '%s' of blank line is invalid (and shouldn't).", BLANK_LINE_REGEX);
            }

            return 0;
        }
        else if (!strcmp(option, "ignore-matching-lines")) {

            if(!argument) {
                exit_help();
                exit_error(NULL, "option '%s' requires an argument", option);
            }

            p->ignore_regex_match = (regex_t*)malloc(sizeof(regex_t)); // regex

            if(regcomp(p->ignore_regex_match, argument, REG_NOSUB | REG_EXTENDED) != 0) { // Compilation
                exit_help();
                exit_error(NULL, "regex '%s' is invalid", argument);
            }

            return 1;
        }
        else if (!strcmp(option, "I")) {

            if(!argument) {
                exit_help();
                exit_error(NULL, "option requires an argument -- '%s'", option);
            }

            p->ignore_regex_match = (regex_t*)malloc(sizeof(regex_t)); // regex

            if(regcomp(p->ignore_regex_match, argument, REG_NOSUB | REG_EXTENDED) != 0) { // Compilation
                exit_help();
                exit_error(NULL, "regex '%s' is invalid", argument);
            }

            return 1;
        }
        else if (!strcmp(option, "text") || !strcmp(option, "a")) {

            if(p->binary) {
                exit_help();
                exit_error(NULL, "conflicting analyse options", NULL);
            }

            p->text = _true;
            return 0;
        }
        else if (!strcmp(option, "binary")) {

            if(p->text) {
                exit_help();
                exit_error(NULL, "conflicting analyse options", NULL);
            }

            p->binary = _true;
            return 0;
        }
        else if (!strcmp(option, "strip-trailing-cr")) {
            p->strip_trailing_cr = _true;
            return 0;
        }
        else if (!strcmp(option, "expand-tabs") || !strcmp(option, "t")) {
            p->expand_tab = _true;
            return 0;
        }
        else if (!strcmp(option, "expand-tabs") || !strcmp(option, "t")) {

            if(!argument) {
                exit_help();
                exit_error(NULL, "option requires an argument -- '%s'", option);
            }

            if((p->size_tab = atoi(argument)) == 0) {
                p->size_tab = 8;
            }

            return 0;
        }
        else if (!strcmp(option, "help")) {
            #ifdef DEBUG
                puts("Help selected");
            #endif
            print_help();
            #ifdef DEBUG
                puts("... diff will now stop\n--------------");
            #endif
            exit(0);
        }
        else if (!strcmp(option, "v") || !strcmp(option, "version")) {
            #ifdef DEBUG
                puts("Version selected");
            #endif
            print_version();
            #ifdef DEBUG
                puts("... diff will now stop\n--------------");
            #endif
            exit(0);
        }
        #ifdef DEBUG
        else if (!strcmp(option, "debug-show-options")) {
            p->d_show_options = _true;
            return 0;
        }
        else if (!strcmp(option, "debug-show-index")) {
            p->d_show_index = _true;
            return 0;
        }
        else if (!strcmp(option, "debug-show-diff")) {
            p->d_show_diff = _true;
            return 0;
        }
        else if (!strcmp(option, "debug-interactive-mode")) {
            p->d_interactive_mode = _true;
            return 0;
        }
        #endif
        else {
            exit_help();
            if(strlen(option) < 2) {
                exit_error(NULL, "invalid option -- '%s'", option); // court
            } else {
                exit_error(NULL, "unrecognized option '%s'", option); // long
            }
        }
    }

    return 0;

}

#ifdef DEBUG
void print_params(Params* parameters) {
	if (parameters == NULL) {
		return;
	}

	printf("Options list : \n");

	printf("Output Style Code : %d\n", parameters->o_style);
	printf("Brief : %d\n", parameters->brief);
	printf("Context : %d\n", parameters->context);

	printf("Width : %u\n", parameters->width);
    if(p->show_regex_function)
	printf("Show function line : 1\n");
	else
	printf("Show function line : 0\n");
	printf("Label 1 : %s\n", parameters->label_1);
	printf("Label 2 : %s\n", parameters->label_2);

	printf("Expand tab : %d\n", parameters->expand_tab);
	printf("Align tab : %d\n", parameters->align_tab);
	printf("Size tab : %d\n", parameters->size_tab);
	printf("Delete first space : %d\n", parameters->suppress_blank_empty);

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
	printf("Ignore tab : %d\n", parameters->ignore_tab_change);
	printf("Ignore end space : %d\n", parameters->ignore_end_space);
	printf("Ignore change space : %d\n", parameters->ignore_space_change);
	printf("Ignore all space : %d\n", parameters->ignore_all_space);
	printf("Ignore blank lines : %d\n", parameters->ignore_blank_lines);

	if(parameters->ignore_regex_match)
        fputs("Ignore regex match : 1\n", stdout);
    else
        fputs("Ignore regex match : 0\n", stdout);

	printf("Type text : %d\n", parameters->text);
	printf("Strip trailing cr : %d\n", parameters->strip_trailing_cr);

	printf("Group format GFMT : %s\n", parameters->group_format_GFMT);
	printf("Line format LFMT : %s\n", parameters->line_format_LFMT);
	printf("Line type format LFMT : %s\n", parameters->line_type_format_LFMT);

	printf("Minimal diference : %d\n", parameters->minimal_diference);
	printf("Horizontal lines : %d\n", parameters->horizontal_lines);
	printf("Speed large files : %d\n", parameters->speed_large_files);

	printf("Show help : %d\n", parameters->show_help);
	printf("Show version : %d\n", parameters->show_version);

	printf("Path left : %s\n", parameters->pathLeft);
	printf("Path right : %s\n", parameters->pathRight);

}
#endif

void free_params_glob(void) {

    if(p) {
        /* Ici libérer tout les char* et cie */

        if(p->ignore_regex_match)
            regfree(p->ignore_regex_match);
        if(p->ignore_blank_lines)
            regfree(p->ignore_blank_lines);
        if(p->show_regex_function)
            regfree(p->show_regex_function);

        free(p);
        p = NULL;
    }

    #ifdef DEBUG
        printf("Exit of the program\n");
    #endif
}
