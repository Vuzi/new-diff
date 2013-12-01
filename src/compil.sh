#!/bin/bash

echo "======== Lancement compilation ========"

if [[ ! -d "_o" ]]; then
	mkdir "_o"
fi

if [[ ! -d "bin" ]]; then
	mkdir "bin"
	mkdir "bin/debug"
	mkdir "bin/release"
fi

if [[ $1 == "-debug" ]]; then
	mode_comp=DEBUG
	path_comp=bin/debug
else
	mode_comp=RELEASE
	path_comp=bin/release
fi


gcc -c main.c -o _o/main.o -Wall -Wextra -O3 -D $mode_comp
gcc -c init/init.c -o _o/init.o -Wall -Wextra -O3 -D $mode_comp
gcc -c init/params.c -o _o/params.o -Wall -Wextra -O3 -D $mode_comp
gcc -c init/paths.c -o _o/paths.o -Wall -Wextra -O3 -D $mode_comp
gcc -c index/index.c -o _o/index.o -Wall -Wextra -O3 -D $mode_comp
gcc -c index/hash.c -o _o/hash.o -Wall -Wextra -O3 -D $mode_comp
gcc -c diff/diff.c -o _o/diff.o -Wall -Wextra -O3 -D $mode_comp
gcc -c diff/smatrix.c -o _o/smatrix.o -Wall -Wextra -O3 -D $mode_comp
gcc -c print/print.c -o _o/print.o -Wall -Wextra -O3 -D $mode_comp
gcc -c err/err.c -o _o/err.o -Wall -Wextra -O3 -D $mode_comp
gcc -Wall -Wextra -O3 -o $path_comp/diff _o/main.o _o/init.o _o/params.o _o/paths.o _o/index.o _o/hash.o _o/diff.o _o/smatrix.o _o/print.o _o/err.o

rm -rf "_o"

echo "========  Compilation terminee ! ========"