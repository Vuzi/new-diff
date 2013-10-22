#define CREATE_PARAMETERS
#include "constant.h"

int main(int argc, char **argv) {
	parameter = initialize_params();

	if (check_params(argc, argv, parameter)) {
		printf("Aucun soucis !\n");
	} else {
		printf("Probleme !\n");
	}
}

