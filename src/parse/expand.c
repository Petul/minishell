/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: copireyr <copireyr@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/04 10:00:45 by copireyr          #+#    #+#             */
/*   Updated: 2024/10/14 10:00:50 by copireyr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "expand.h"
#include "libft.h"

static char			*expand_str(t_arena arena, t_list *env, const char *end, int exit_code);
static int			find_next_expandable(const char *str);
static char			*val(t_list *env, const char *key, size_t length_key);
static char			*get_exit_code_as_cstr(t_arena arena, int exit_code);

/*TODO: Check return values for ENOMEM*/
void	expand(t_ast_node *ast, t_arena arena, t_list *env, int exit_code)
{
	size_t	i;

	if (!ast)
		return ;
	i = 0;
	while (i < ast->n_children)
	{
		if (ast->children[i]->type == AST_WORD)
		{
			ast->children[i]->token.value = expand_str(
					arena, env, ast->children[i]->token.value, exit_code);
			ast->children[i]->token.size = ft_strlen(
					ast->children[i]->token.value);
		}
		expand(ast->children[i], arena, env, exit_code);
		i++;
	}
}

/* TODO: $? */
static char	*expand_str(t_arena arena, t_list *env, const char *end, int exit_code)
{
	t_string_vector	vec;
	const char		*start;

	vec = (t_string_vector){0};
	while (*end)
	{
		vec = realloc_maybe(arena, vec);
		if (!vec.strings)
			return (NULL);
		start = end;
		end += find_next_expandable(end);
		vec.strings[vec.count++] = ft_arena_strndup(arena, start, end - start);
		if (!vec.strings[vec.count - 1])
			return (NULL);
		if (*end == '$')
		{
			start = ++end;
			if (*start == '?')
			{
				vec.strings[vec.count++] = get_exit_code_as_cstr(arena, exit_code);
				if (!vec.strings[vec.count - 1])
					return (NULL);
				end++;
				continue ;
			}
			while (ft_isalnum(*end) || *end == '_')
				end++;
			vec.strings[vec.count++] = val(env, start, end - start);
		}
	}
	return (ft_arena_strjoin(arena, vec.strings, vec.count));
}

static char *get_exit_code_as_cstr(t_arena arena, int exit_code)
{
	char *code;
	char *ret;

	code = ft_itoa(exit_code);
	if (!code)
		return (NULL);
	ret = ft_arena_strndup(arena, code, ft_strlen(code) + 1);
	free(code);
	if (!ret)
		return (NULL);
	return (ret);
}

t_string_vector	realloc_maybe(t_arena arena, t_string_vector vec)
{
	char	**tmp;

	if (vec.count < vec.capacity)
		return (vec);
	if (!vec.capacity)
		vec.capacity = 8;
	vec.capacity = 2 * vec.capacity;
	tmp = arena_calloc(arena, vec.capacity, sizeof(char *));
	if (tmp)
		ft_memcpy(tmp, vec.strings, sizeof(char *) * vec.count);
	vec.strings = tmp;
	return (vec);
}

/*
 * static variable is dangerous here, because the state will NOT
 * be wiped clean when we parse the next command. However we know
 * from the tokenizer that double quotes only occur in pairs, so
 * expand_anyway should always be false when we are done expanding.
 */

static int	find_next_expandable(const char *str)
{
	static bool	expand_anyway = false;
	int			i;

	i = 0;
	while (str[i] && str[i] != '$')
	{
		if (str[i] == '"')
			expand_anyway = !expand_anyway;
		if (str[i] == '\'' && !expand_anyway)
			while (str[++i] != '\'')
				;
		i++;
	}
	return (i);
}

static char	*val(t_list *env, const char *key, size_t length_key)
{
	char	*curr;
	size_t	i;

	while (env && length_key)
	{
		i = 0;
		curr = env->content;
		while (curr[i] != '=')
			i++;
		if (i == length_key && !ft_memcmp(key, curr, length_key))
			return (curr + i + 1);
		env = env->next;
	}
	return ("");
}
