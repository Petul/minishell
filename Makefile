# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: copireyr <copireyr@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/08/27 12:07:56 by copireyr          #+#    #+#              #
#    Updated: 2024/11/19 15:44:08 by copireyr         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

.DEFAULT_GOAL := all
CC := cc

DEBUG := 0
CPPFLAGS := -I./include/ -I./libft/include
LDFLAGS := -lreadline -L./libft/ -lft
CFLAGS := -Wall -Wextra -Werror -MMD -MP
NAME := minishell

ifeq ($(shell uname -s), Darwin)
	include macOS.mk
endif

src := ./src/main.c ./src/environment.c ./src/signals.c

parse := $(addprefix ./src/parse/, ast.c ast_utils.c ast_find_next_logical_token.c ast_get_highest_ast_type.c ast_create_command_node.c parse.c tokenize.c tokenize_utils.c \
		 expand.c glob.c split_words.c remove_quotes.c glob_quotes.c)
builtins := $(addprefix ./src/builtins/, builtins.c builtin_env.c builtin_export.c builtin_unset.c builtin_pwd.c builtin_cd.c builtin_echo.c builtin_exit.c)
execute := $(addprefix ./src/execute/, execute_ast.c make_command.c pipe.c run_builtin.c logicals.c minishell_execve.c execute_utils.c execute_single_cmd.c execute_cmd.c execute_pipeline.c make_command_utils.c handle_redir.c heredoc.c )
src += $(parse) $(builtins) $(execute)

obj := $(src:./src/%.c=./obj/%.o)
lib := ./libft/libft.a

./obj/%.o: ./src/%.c
	@mkdir -p $(@D)
	$(CC) -D DEBUG=$(DEBUG) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(lib):
	make -j4 -C ./libft/

$(NAME): $(lib) $(obj)
	$(CC) $^ -o $@ $(LDFLAGS)

.PHONY: all
all: $(NAME)

.PHONY: clean
clean:
	$(RM) -r ./obj
	make -C ./libft/ clean

.PHONY: fclean
fclean: clean
	$(RM) $(NAME)
	make -C ./libft/ fclean

.PHONY: re
re:
	@make -s fclean
	@make -s -j

.PHONY: run
run: $(NAME)
	./$(NAME)

.PHONY: val
val: $(NAME)
	valgrind --suppressions=readline.supp -s ./$(NAME)


.PHONY: test
test: $(NAME)
	cd minishell_tester/ && ./tester
-include $(obj:.o=.d)
