#include "constant.h"
#include "params.h"

Params* initialize_params() {
	Params* return_params = NULL;

	return_params = (Params*) calloc(1, sizeof(Params));
	
	return_params->brief = (Mode) NORMAL;

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
	unsigned int result = 0;

	if (parameters != NULL) {

		if (argc < 2) {
			return 1;
		}

		for (i = 1; i < argc; i += 1) {
			printf("Reading a parameter : %s\n", argv[i]);
			result = read_param(argv[i], parameters);

			if (result == 1) {
				/* Last paramters "--PARAM[=VALUE] return 1" */
				return 1;
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

int read_param(char *parameter, Params* parameters) {

	char test_dash = '-';
	char test_end = '\0';
	char test_space = ' ';
	char test_equals = '=';

	int count = 0;
	int size = strlen(parameter);

	unsigned short swap = 0;
	char *param = NULL;
	char* value = NULL;

	if (parameter[0] == test_dash && parameter[1] == test_dash) {
		if (size == 2) {
			printf("Double entry read !\n");
			if (parameters->pathLeft == NULL) {
				parameters->pathLeft = (char*) malloc(sizeof(char) );
				parameters->pathLeft[0] = '-';
			}
			if (parameters->pathRight == NULL) {
				parameters->pathRight = (char*) malloc(sizeof(char) );
				parameters->pathRight[0] = '-';
			}
		}
		printf("Long params :\n");
		count = 2;
		while (parameter[count] != test_end) {
			if (swap) {
				/* Add a letter for a value */
				printf("%c", toUpper(parameter[count]));
			} else {
				if (parameter[count] == test_equals) {
					printf(" <-> ");
					swap = 1;
				} else {
					/* Add a letter for a param */
					printf("%c", toLower(parameter[count]));
				}
			}
			count += 1;
		}
		printf("\nEnd read a long params\n");
		return 1;
	} else if (parameter[0] == test_dash) {
		if (size == 1) {
			/* Entry read */
			printf("Simple entry read !\n");

			if (parameters->pathLeft != NULL) {
				parameters->pathRight = (char*) malloc(sizeof(char) );
				parameters->pathRight[0] = '-';
			} else {
				parameters->pathLeft = (char*) malloc(sizeof(char) );
				parameters->pathLeft[0] = '-';
			}
		}

		if (size < 3) {
			
		} else {
			if (parameter[2] == test_space) {
			} else {

			}
		}
	} else {
		printf("Add a path for ... ");
		printf("\n%s\n", parameter);
	}

	return 0;
}
