text_editor:
	clang -fsanitize=address -ggdb3 -gdwarf-4 -O0 -Qunused-arguments -std=gnu11 -Wall -Werror -Wextra -Wno-gnu-folding-constant -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wshadow -c -o text_editor.o text_editor.c
	clang -fsanitize=address -ggdb3 -gdwarf-4 -O0 -Qunused-arguments -std=gnu11 -Wall -Werror -Wextra -Wno-gnu-folding-constant -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wshadow -c -o starter.o starter.c
	clang -fsanitize=address -ggdb3 -gdwarf-4 -O0 -Qunused-arguments -std=gnu11 -Wall -Werror -Wextra -Wno-gnu-folding-constant -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wshadow -o text_editor starter.o text_editor.o -lm

delete_file:
	rm -f text_editor
	rm -f starter.o
	rm -f text_editor.o