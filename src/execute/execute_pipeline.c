/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execute_pipeline.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 10:22:05 by pleander          #+#    #+#             */
/*   Updated: 2024/11/14 10:24:39 by pleander         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "execute.h"

static void	init_pipeline(t_command_context *con, t_list *env, t_ast_node *ast,
		t_arena arena)
{
	con->env = env;
	con->n_children = calculate_n_pipes(ast) + 1;
	con->pipes = make_pipes(con->n_children - 1, arena);
	con->cur_child = 0;
}

static pid_t	do_forking(t_command_context *con, int prev_exit, t_arena arena)
{
	pid_t	pid;

	pid = fork();
	if (pid == 0)
		execute_cmd(con, arena, prev_exit);
	return (pid);
}

static void	cleanup_pipeline(t_command_context *con, pid_t *child_pids,
			t_shell_status *status)
{
	close_pipes(con->pipes, con->n_children - 1);
	status->exit_code = wait_for_children(child_pids, con->n_children);
	status->should_exit = false;
}

t_shell_status	execute_pipeline(
		t_ast_node *ast, t_list *env, t_arena arena, int prev_exit)
{
	t_command_context	con;
	pid_t				*child_pids;
	t_shell_status		status;

	init_pipeline(&con, env, ast, arena);
	child_pids = arena_alloc(arena, (con.n_children) * sizeof(pid_t));
	if (!child_pids || !con.pipes)
		return ((t_shell_status){.exit_code = -1});
	while (ast->type == AST_PIPELINE || ast->type == AST_COMMAND)
	{
		if (ast->type == AST_PIPELINE)
			con.ast = ast->children[0];
		else if (ast->type == AST_COMMAND)
			con.ast = ast;
		child_pids[con.cur_child] = do_forking(&con, prev_exit, arena);
		if (child_pids[con.cur_child] == -1)
			return ((t_shell_status){.exit_code = -1, .should_exit = true});
		if (ast->type == AST_COMMAND)
			break ;
		ast = ast->children[1];
		con.cur_child++;
	}
	cleanup_pipeline(&con, child_pids, &status);
	return (status);
}