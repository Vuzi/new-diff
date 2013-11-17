@echo "======== Lancement compilation ========"
@mkdir _o
@mkdir bin
@mkdir "bin/debug"
@gcc -c main.c -o _o/main.o -Wall -Wextra -O3
@gcc -c path_test.c -o _o/path_test.o -Wall -Wextra -O3
@gcc -c diff_dir/diff.c -o _o/diff_dir.o -Wall -Wextra -O3
@gcc -c diff_file/diff.c -o _o/diff_file.o -Wall -Wextra -O3
@gcc -c diff_file/diff_list.c -o _o/diff_list.o -Wall -Wextra -O3
@gcc -c diff_file/index.c -o _o/index.o -Wall -Wextra -O3
@gcc -c diff_file/line.c -o _o/line.o -Wall -Wextra -O3
@gcc -c err/err.c -o _o/err.o -Wall -Wextra -O3
@gcc -c params/params.c -o _o/params.o -Wall -Wextra -O3
@gcc -Wall -Wextra -O3 -o bin/debug/diff.exe _o/main.o _o/path_test.o _o/diff_dir.o _o/diff_file.o _o/diff_list.o _o/index.o _o/line.o _o/err.o _o/params.o lib/libregex.dll.a
@del _o /Q
@rmdir _o /q /s
@echo "========  Compilation terminee ! ========"
@pause