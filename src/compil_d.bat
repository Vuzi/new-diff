@echo "======== Lancement compilation ========"
@mkdir _o
@mkdir bin
@mkdir "bin/debug"
@gcc -c main.c -o _o/main.o -Wall -Wextra -O3 -D DEBUG
@gcc -c init/init.c -o _o/init.o -Wall -Wextra -O3 -D DEBUG
@gcc -c init/params.c -o _o/params.o -Wall -Wextra -O3 -D DEBUG
@gcc -c init/paths.c -o _o/paths.o -Wall -Wextra -O3 -D DEBUG
@gcc -c index/index.c -o _o/index.o -Wall -Wextra -O3 -D DEBUG
@gcc -c index/hash.c -o _o/hash.o -Wall -Wextra -O3 -D DEBUG
@gcc -c diff/diff.c -o _o/diff.o -Wall -Wextra -O3 -D DEBUG
@gcc -c diff/smatrix.c -o _o/smatrix.o -Wall -Wextra -O3 -D DEBUG
@gcc -c print/print.c -o _o/print.o -Wall -Wextra -O3 -D DEBUG
@gcc -c err/err.c -o _o/err.o -Wall -Wextra -O3 -D DEBUG
@gcc -Wall -Wextra -O3 -o bin/debug/diff.exe _o/main.o _o/init.o _o/params.o _o/paths.o _o/index.o _o/hash.o _o/diff.o _o/smatrix.o _o/print.o _o/err.o lib/libregex.dll.a
@del _o /Q
@rmdir _o /q /s
@echo "========  Compilation terminee ! ========"
@pause