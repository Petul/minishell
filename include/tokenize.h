/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenize.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: copireyr <copireyr@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/26 14:29:23 by copireyr          #+#    #+#             */
/*   Updated: 2024/10/01 09:37:22 by copireyr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TOKENIZE_H
# define TOKENIZE_H

# include <stdlib.h>
# include <limits.h>
# include "libft.h"

enum e_type
{
	AST_WORD,
	AST_COMMAND,
	AST_APPEND,
	AST_REDIRECT_IN,
	AST_REDIRECT_OUT,
	AST_HEREDOC,
	AST_LOGICAL_AND,
	AST_LOGICAL_OR,
	AST_PIPE,
	AST_ERROR,
	AST_META,
	AST_TOKENIZE_SPACE,
	AST_END,
	AST_NUM_TYPES,
};

typedef struct s_ast
{
	enum e_type		type;
	char			*value;
	struct s_ast	**nodes;
}	t_ast;

typedef struct s_token
{
	enum e_type	type;
	const char	*value;
	size_t		size;
}	t_token;

void	tokenize_test(void);
t_token	*tokenize(const char *str);
void	tokenize_show(t_token token);
void	tokenize_show_tokens(t_token *xs);
const char	*ast_show_type(enum e_type type);

void	parse(t_token *xs);

#endif /* TOKENIZE_H */
