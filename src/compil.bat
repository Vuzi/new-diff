@echo "======== Lancement compilation ========"
@mkdir _o
@mkdir bin

@set mode_comp="RELEASE"
@set path_comp="bin/release"

@if "%1" == "-debug" (
	@set mode_comp="DEBUG"
	@set path_comp="bin/debug"
)

@mkdir %path_comp%

@gcc -c main.c -o _o/main.o -Wall -Wextra -O3 -D %mode_comp%
@gcc -c init/init.c -o _o/init.o -Wall -Wextra -O3 -D %mode_comp%
@gcc -c init/params.c -o _o/params.o -Wall -Wextra -O3 -D %mode_comp%
@gcc -c init/paths.c -o _o/paths.o -Wall -Wextra -O3 -D %mode_comp%
@gcc -c index/index.c -o _o/index.o -Wall -Wextra -O3 -D %mode_comp%
@gcc -c index/hash.c -o _o/hash.o -Wall -Wextra -O3 -D %mode_comp%
@gcc -c diff/diff.c -o _o/diff.o -Wall -Wextra -O3 -D %mode_comp%
@gcc -c diff/snake.c -o _o/snake.o -Wall -Wextra -O3 -D %mode_comp%
@gcc -c print/print.c -o _o/print.o -Wall -Wextra -O3 -D %mode_comp%
@gcc -c err/err.c -o _o/err.o -Wall -Wextra -O3 -D %mode_comp%
@gcc -c string/_string.c -o _o/_string.o -Wall -Wextra -O3 -D %mode_comp%
@gcc -Wall -Wextra -O3 -o %path_comp%/diff.exe _o/main.o _o/init.o _o/params.o _o/paths.o _o/index.o _o/hash.o _o/diff.o _o/snake.o _o/print.o _o/err.o _o/_string.o lib/libregex.dll.a
@del _o /Q
@rmdir _o /q /s
@echo "========  Compilation terminee ! ========"
@pause
