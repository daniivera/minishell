all: minishell

minishell: minishell.c
	gcc -Wall -Wextra minishell.c -o minishell

clean:
	rm -f minishell

.PHONY: all clean