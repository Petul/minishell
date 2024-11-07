/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logicals.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 09:19:39 by pleander          #+#    #+#             */
/*   Updated: 2024/11/07 14:37:44 by pleander         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "arena.h"
#include "ast.h"
#include "execute.h"
#include "tokenize.h"

t_shell_status	execute_logicals(
		t_ast_node *ast, t_list *env, t_arena arena, int prev_exit)
{
	t_ast_node		*cur;
	t_shell_status	status;
	
	cur = ast;
	status = execute_ast(cur->children[0], env, arena, prev_exit);
	while (cur->type == AST_LOGICAL)
	{
		if (cur->token.type == TOK_LOGICAL_AND)
		{
			if (cur->children[1]->type == AST_LOGICAL)
			{
				status.exit_code = !((status.exit_code == 0) && (execute_ast(cur->children[1]->children[0], env, arena, prev_exit).exit_code == 0));
			}
			else
			{
				status.exit_code = !((status.exit_code == 0) && (execute_ast(cur->children[1], env, arena, prev_exit).exit_code == 0));
			}
		}
		if (cur->token.type == TOK_LOGICAL_OR)
		{
			if (cur->children[1]->type == AST_LOGICAL)
			{
				status.exit_code = !((status.exit_code == 0) 
					|| (execute_ast(cur->children[1]->children[0], env, arena, prev_exit).exit_code == 0));
			}
			else
			{
				status.exit_code = !((status.exit_code == 0) || 
					(execute_ast(cur->children[1], env, arena, prev_exit).exit_code == 0));
			}
		}
		if (status.exit_code != 0)
			break ;
		cur = cur->children[1];
	}
	return (status);
}
