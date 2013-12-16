#include "params.h"
#include "../print/print.h" // Fonction print

/* == Variables globales == */
Params *p = NULL;
suint diff_stderr_show_help = 0;


static void set_file_name(char *name);
static void set_output_style(output_style style);
static void set_context(char* val);
static void set_width(char* val);


/* ===============================================
                     initialize_params

    Initialize the global struct containing all
    the parameters.
   =============================================== */
void initialize_params(void) {
    uint i = 0;
	p = (Params*) diff_xcalloc(1, sizeof(Params));

	p->o_style = NOT_SELECTED;
	p->context = 3;

	p->width = 130;
	p->size_tab = 8;

	p->show_regex_function = NULL;

	p->exclude_pattern = NULL;
	p->exclude_from = NULL;

	p->start_compare_file_in_dir = NULL;

	p->ignore_blank_lines = NULL;
	p->ignore_regex_match = NULL;

	p->ifdef = NULL;

    for(; i < 2; i++) {
        p->paths[i] = NULL;
        p->labels[i] = NULL;
    }

	p->argc = 0;
	p->argv = NULL;

}


/* ===============================================
                     make_params

    Analyse the array argv to find all the
    parameters.
    ----------------------------------------------
    int argc    : size of argv
    char **argv : array containing the options
   =============================================== */
void make_params(int argc, char **argv) {

    int i = 1, j = 0, arg_length;
    short int no_more_op = 0;

    char short_tmp[2] = {0};
    char *arg_tmp = NULL;

    // For each param
    for (i = 1; i < argc; i++) {

        arg_length = diff_strlen(argv[i]);

        // Start with a dash
        if(!no_more_op && arg_length > 0 && argv[i][0] == '-') {
            // Second dash : long argument
            if(arg_length > 1 && argv[i][1] == '-') {
                // If just double dash, no more args
                if(arg_length == 2) {
                    no_more_op = 1;
                }
                // Long argument
                else {

                    if(argv[i][2] == '=') {
                        exit_help();
                        exit_error(NULL, "option '%s' is ambigous", argv[i]);
                    }

                    arg_tmp = diff_strchr(argv[i]+2, (int)'=');

                    if(arg_tmp) {
                        *arg_tmp = '\0';
                        arg_tmp++;
                        make_param(argv[i]+2, arg_tmp);
                    } else {
                        if(argc < i+1)
                            i += make_param(argv[i]+2, argv[i+1]);
                        else
                            make_param(argv[i]+2, NULL);
                    }
                }
            }
            // Only one dash, stdin
            else if(arg_length == 1) {
                set_file_name(argv[i]);
            }
            // Short args
            else {
                // More than one (or values)
                if(arg_length > 2) {
                    for(j = 1; j < arg_length; j++) {
                        short_tmp[0] = argv[i][j];

                        if(j+1 < arg_length) {
                            if(make_param(short_tmp, argv[i]+j+1)) // argument + line ending
                                j = arg_length-1;
                        } else if(i+1 < argc)
                            i += make_param(short_tmp, argv[i+1]); // argument + next
                        else
                            make_param(short_tmp, NULL);
                    }
                }
                // One arg
                else {
                    if(i+1 < argc)
                        i += make_param(argv[i]+1, argv[i+1]);
                    else
                        make_param(argv[i]+1, NULL);
                }
            }
        }
        // Node dash, filename
        else {
            set_file_name(argv[i]);
        }

    }

    // If no output, set to regular
    if(p->o_style == NOT_SELECTED) {
        if(p->show_regex_function)
            p->o_style = CONTEXT;
        else
            p->o_style = REGULAR;
    }

    // Check if two paths are given
    if(!(p->paths[0])) {
        exit_help();
        exit_error(NULL, "missing operand after 'diff'", NULL);
    } else if (!(p->paths[1])) {
        exit_help();
        exit_error(NULL, "missing operand after '%s'", p->paths[0]);
    }
}


/* ===============================================
                     set_file_name

    Set the filename to name. If more than two
    filenames are given, an error is generated.
    ----------------------------------------------
    char *name : name of the file
   =============================================== */
static void set_file_name(char *name) {

    #ifdef DEBUG
        printf("Filename : %s\n--------\n", name);
    #endif

    if(p) {
        if(!(p->paths[0])) {
            p->original_paths[0] = p->paths[0] = name;
        } else if(!(p->paths[1])) {
            p->original_paths[1] = p->paths[1] = name;
        } else {
            exit_help();
            exit_error(NULL, "extra operand '%s'", name);
        }
    }
}


/* ===============================================
                     set_output_style

    Set the output style to style. If an output
    style is already selected, an error is
    generated.
    ----------------------------------------------
    output_style style : new output style
   =============================================== */
static void set_output_style(output_style style) {
    if(p->o_style != NOT_SELECTED && style != p->o_style) {
        exit_help();
        exit_error(NULL, "conflicting output style options", NULL);
    } else
        p->o_style = style;
}


/* ===============================================
                     set_context

    Set the context to val. If val refers to a
    value below zero, and error is generated.
    ----------------------------------------------
    char* val : string of the value
   =============================================== */
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


/* ===============================================
                     set_width

    Set the context to val. If val refers to a
    value below or equal to zero, and error is
    generated.
    ----------------------------------------------
    char* val : string of the value
   =============================================== */
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


/* ===============================================
                     make_param

    Analyse a parameter and its argument
    ----------------------------------------------
    char* option   : option to analyse
    char* argument : argument to that option
    ----------------------------------------------
    Return 1 whe argument is used, 0 otherwise.
   =============================================== */
int make_param(char* option, char* argument) {

    #ifdef DEBUG
        printf("Option : %s\nArgument : %s\n--------\n", option, argument);
    #endif

    if(p) {
        if (!diff_strcmp(option, "normal")) {
            set_output_style(REGULAR);
            return 0;
        }
        else if (!diff_strcmp(option, "brief") || !diff_strcmp(option, "q")) {
            p->brief = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "report-identical-files") || !diff_strcmp(option, "s")) {
            p->report_identical_files = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "c")) {
            set_output_style(CONTEXT);
            return 0;
        }
        else if (!diff_strcmp(option, "C")) {

            if(!argument) {
                exit_help();
                exit_error(NULL, "option requires an argument -- '%s'", option);
            }

            set_output_style(CONTEXT);
            set_context(argument);
            return 1;

        }
        else if (!diff_strcmp(option, "context")) {

            set_output_style(CONTEXT);

            if(argument) {
                set_context(argument);
                return 1;
            }

            return 0;
        }
        else if (!diff_strcmp(option, "u")) {
            set_output_style(UNIFIED);
        }
        else if (!diff_strcmp(option, "U")) {

            if(!argument) {
                exit_help();
                exit_error(NULL, "option requires an argument -- '%s'", option);
            }

            set_output_style(UNIFIED);
            set_context(argument);
            return 1;

        }
        else if (!diff_strcmp(option, "unified")) {

            set_output_style(UNIFIED);

            if(argument) {
                set_context(argument);
                return 1;
            }

            return 0;
        }
        else if (!diff_strcmp(option, "ed") || !diff_strcmp(option, "e")) {
            set_output_style(EDIT_SCRIPT);
            return 0;
        }
        else if (!diff_strcmp(option, "rcs") || !diff_strcmp(option, "n")) {
            set_output_style(RCS);
            return 0;
        }
        else if (!diff_strcmp(option, "ifdef")) {

            set_output_style(IFDEF);

            if(argument) {
                p->ifdef = argument;
                return 1;
            } else {
                exit_help();
                exit_error(NULL, "option '%s' requires an argument", option);
            }

            return 0;
        }
        else if (!diff_strcmp(option, "side-by-side") || !diff_strcmp(option, "y")) {
            set_output_style(COLUMNS);
            return 0;
        }
        else if (!diff_strcmp(option, "width") || !diff_strcmp(option, "W")) {

            if(!argument) {
                exit_help();
                exit_error(NULL, "option requires an argument -- '%s'", option);
            }

            set_width(argument);
            return 1;
        }
        else if (!diff_strcmp(option, "left-column")) {
            p->left_column = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "suppress-common-lines")) {
            p->suppress_common_lines = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "recursive") || !diff_strcmp(option, "r")) {
            p->recursive_dir = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "new-file") || !diff_strcmp(option, "N")) {
            p->new_file = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "ignore-file-name-case")) {
            p->ignore_case_filename = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "no-ignore-file-name-case")) {
            p->ignore_case_filename = _false;
            return 0;
        }
        else if (!diff_strcmp(option, "show-c-function") || !diff_strcmp(option, "p")) {

            if (p->show_regex_function) {
                exit_help();
                exit_error(NULL, "conflicting type of matching function to show", option);
            }

            p->show_regex_function = (regex_t*)diff_xmalloc(sizeof(regex_t)); // regex

            if(regcomp(p->show_regex_function, C_FUNCTION_REGEX, REG_NOSUB | REG_EXTENDED) != 0) { // Compilation
                exit_help();
                exit_error(NULL, "regex '%s' of c function is invalid (and shouldn't).", C_FUNCTION_REGEX);
            }

            return 0;
        }
        else if (!diff_strcmp(option, "show-function-line") || !diff_strcmp(option, "F")) {

            if(!argument) {
                exit_help();
                exit_error(NULL, "option '%s' requires an argument", option);
            } else if (p->show_regex_function) {
                exit_help();
                exit_error(NULL, "conflicting type of matching function to show", option);
            }

            p->show_regex_function = (regex_t*)diff_xmalloc(sizeof(regex_t)); // regex

            if(regcomp(p->show_regex_function, argument, REG_NOSUB | REG_EXTENDED) != 0) { // Compilation
                exit_help();
                exit_error(NULL, "regex '%s' is invalid", argument);
            }

            return 1;
        }
        else if (!diff_strcmp(option, "label") || !diff_strcmp(option, "L") ) {

            if(!argument) {
                exit_help();
                exit_error(NULL, "option '%s' requires an argument", option);
            }

            if(!p->labels[0])
                p->labels[0] = argument;
            else if(!p->labels[1])
                p->labels[1] = argument;
            else
                exit_error(NULL, "more than two labels given '%s'", option);
            return 1;
        }
        else if (!diff_strcmp(option, "suppress-blank-empty")) {
            p->suppress_blank_empty = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "ignore-case") || !diff_strcmp(option, "i")) {
            p->ignore_case = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "ignore-tab-expansion") || !diff_strcmp(option, "E")) {
            p->ignore_tab_change = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "ignore-trailing-space") || !diff_strcmp(option, "Z")) {
            p->ignore_end_space = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "ignore-space-change") || !diff_strcmp(option, "b")) {
            p->ignore_space_change = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "ignore-all-space") || !diff_strcmp(option, "w")) {
            p->ignore_all_space = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "ignore-blank-lines") || !diff_strcmp(option, "B")) {

            p->ignore_blank_lines = (regex_t*)diff_xmalloc(sizeof(regex_t)); // regex

            if(regcomp(p->ignore_blank_lines, BLANK_LINE_REGEX, REG_NOSUB | REG_EXTENDED) != 0) { // Compilation
                exit_help();
                exit_error(NULL, "regex '%s' of blank line is invalid (and shouldn't).", BLANK_LINE_REGEX);
            }

            return 0;
        }
        else if (!diff_strcmp(option, "ignore-matching-lines")) {

            if(!argument) {
                exit_help();
                exit_error(NULL, "option '%s' requires an argument", option);
            }

            p->ignore_regex_match = (regex_t*)diff_xmalloc(sizeof(regex_t)); // regex

            if(regcomp(p->ignore_regex_match, argument, REG_NOSUB | REG_EXTENDED) != 0) { // Compilation
                exit_help();
                exit_error(NULL, "regex '%s' is invalid", argument);
            }

            return 1;
        }
        else if (!diff_strcmp(option, "I")) {

            if(!argument) {
                exit_help();
                exit_error(NULL, "option requires an argument -- '%s'", option);
            }

            p->ignore_regex_match = (regex_t*)diff_xmalloc(sizeof(regex_t)); // regex

            if(regcomp(p->ignore_regex_match, argument, REG_NOSUB | REG_EXTENDED) != 0) { // Compilation
                exit_help();
                exit_error(NULL, "regex '%s' is invalid", argument);
            }

            return 1;
        }
        else if (!diff_strcmp(option, "text") || !diff_strcmp(option, "a")) {

            if(p->binary) {
                exit_help();
                exit_error(NULL, "conflicting analyse options", NULL);
            }

            p->text = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "binary")) {

            if(p->text) {
                exit_help();
                exit_error(NULL, "conflicting analyse options", NULL);
            }

            p->binary = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "strip-trailing-cr")) {
            p->strip_trailing_cr = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "expand-tabs") || !diff_strcmp(option, "t")) {
            p->expand_tab = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "expand-tabs") || !diff_strcmp(option, "t")) {

            if(!argument) {
                exit_help();
                exit_error(NULL, "option requires an argument -- '%s'", option);
            }

            if((p->size_tab = atoi(argument)) == 0) {
                p->size_tab = 8;
            }

            return 0;
        }
        else if (!diff_strcmp(option, "help")) {
            #ifdef DEBUG
                puts("Help selected");
            #endif
            print_help();
            #ifdef DEBUG
                puts("... diff will now stop\n--------------");
            #endif
            exit(0);
        }
        else if (!diff_strcmp(option, "v") || !diff_strcmp(option, "version")) {
            #ifdef DEBUG
                puts("Version selected");
            #endif
            print_version();
            #ifdef DEBUG
                puts("... diff will now stop\n--------------");
            #endif
            exit(0);
        }
        else if (!diff_strcmp(option, "use-matrix-lcs")) {
            p->use_matrix_lcs = _true;
            return 0;
        }
        #ifdef DEBUG
        else if (!diff_strcmp(option, "debug-show-options")) {
            p->d_show_options = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "debug-show-index")) {
            p->d_show_index = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "debug-show-diff")) {
            p->d_show_diff = _true;
            return 0;
        }
        else if (!diff_strcmp(option, "debug-interactive-mode")) {
            p->d_interactive_mode = _true;
            return 0;
        }
        #endif
        else {
            exit_help();
            if(diff_strlen(option) < 2) {
                exit_error(NULL, "invalid option -- '%s'", option); // court
            } else {
                exit_error(NULL, "unrecognized option '%s'", option); // long
            }
        }
    }

    return 0;

}


#ifdef DEBUG
/* ===============================================
                     print_params

    Display the content of the struct params
    ----------------------------------------------
    Params *parameters : to display
   =============================================== */
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
	printf("Label 1 : %s\n", parameters->labels[0]);
	printf("Label 2 : %s\n", parameters->labels[1]);

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
	if(parameters->ignore_blank_lines)
        printf("Ignore blank lines : 1\n");
    else
        printf("Ignore blank lines : 0\n");

	if(parameters->ignore_regex_match)
        fputs("Ignore regex match : 1\n", stdout);
    else
        fputs("Ignore regex match : 0\n", stdout);

	printf("Type text : %d\n", parameters->text);
	printf("Strip trailing cr : %d\n", parameters->strip_trailing_cr);

	printf("Minimal diference : %d\n", parameters->minimal_diference);
	printf("Horizontal lines : %d\n", parameters->horizontal_lines);
	printf("Speed large files : %d\n", parameters->speed_large_files);

	printf("Show help : %d\n", parameters->show_help);
	printf("Show version : %d\n", parameters->show_version);

	printf("Path 1 : %s\n", parameters->paths[0]);
	printf("Path 2 : %s\n", parameters->paths[1]);

}
#endif


/* ===============================================
                 free_params_glob

    Display the content of the struct params
    ----------------------------------------------
    Params *parameters : to display
   =============================================== */
void free_params_glob(void) {

    if(p) {
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
