/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execute_single_cmd.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 08:35:59 by pleander          #+#    #+#             */
/*   Updated: 2024/11/20 13:46:48 by copireyr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "execute.h"
#include "libft.h"
#include "minishell.h"
#include "signals.h"

static void	restore_fds(int orig_fds[2])
{
	if (orig_fds[0] > -1)
	{
		close(STDIN_FILENO);
		dup2(orig_fds[0], STDIN_FILENO);
		close(orig_fds[0]);
	}
	if (orig_fds[1] > -1)
	{
		close(STDOUT_FILENO);
		dup2(orig_fds[1], STDOUT_FILENO);
		close(orig_fds[1]);
	}
}

static int	init_fds_and_cmd(int orig_fds[2], t_command *cmd,
		t_command_context *con, t_arena arena)
{
	orig_fds[0] = -1;
	orig_fds[1] = -1;
	cmd->infile_fd = -1;
	cmd->outfile_fd = -1;
	if (con->pipes && con->cur_child > 0)
		cmd->infile_fd = con->pipes[con->cur_child - 1][0];
	if (con->pipes && con->cur_child != con->n_children - 1)
		cmd->outfile_fd = con->pipes[con->cur_child][1];
	return (make_command(cmd, con->ast, con->env, arena));
}

static t_shell_status	execute_builtin_cmd(
		t_command_context *con, t_arena arena, int prev_exit)
{
	t_command		cmd;	
	int				orig_fds[2];
	t_shell_status	status;
	int				code;

	code = init_fds_and_cmd(orig_fds, &cmd, con, arena);
	if (code == -42)
		return ((t_shell_status){.exit_code = 130});
	if (code < 0)
		return ((t_shell_status){.exit_code = 1});
	if (cmd.infile_fd > -1)
	{
		orig_fds[0] = dup(STDIN_FILENO);
		dup2(cmd.infile_fd, STDIN_FILENO);
		close(cmd.infile_fd);
	}
	if (cmd.outfile_fd > -1)
	{
		orig_fds[1] = dup(STDOUT_FILENO);
		dup2(cmd.outfile_fd, STDOUT_FILENO);
		close(cmd.outfile_fd);
	}
	status = run_builtin(cmd.path, cmd.args, &con->env, prev_exit);
	restore_fds(orig_fds);
	return (status);
}

static int	init_cmd(t_command_context *con, t_arena arena)
{
	int	code;

	if (con->pipes && con->cur_child > 0)
		con->cmd.infile_fd = con->pipes[con->cur_child - 1][0];
	if (con->pipes && con->cur_child != con->n_children - 1)
		con->cmd.outfile_fd = con->pipes[con->cur_child][1];
	code = make_command(&con->cmd, con->ast, con->env, arena);
	if (code < 0)
		con->child_should_exit = 1;
	return (code);
}

t_shell_status	execute_single_command(
		t_ast_node *ast, t_list *env, t_arena arena, int prev_exit)
{
	t_command_context	con;
	pid_t				*child_pids;

	con = (t_command_context){{NULL, NULL, NULL, NULL, -1, -1},
		ast, env, NULL, 0, 1, 0, false};
	if (is_builtin(con.ast->children[0]->token.value))
		return (execute_builtin_cmd(&con, arena, prev_exit));
	child_pids = arena_alloc(arena, (con.n_children) * sizeof(pid_t));
	if (!child_pids)
		return ((t_shell_status){.exit_code = -1});
	if (init_cmd(&con, arena) == -42)
		return ((t_shell_status){false, true, 130});
	child_pids[0] = fork();
	if (child_pids[0] == 0)
		execute_cmd(&con.cmd, &con, arena, prev_exit);
	close_fd_if_open(&con.cmd.infile_fd);
	close_fd_if_open(&con.cmd.outfile_fd);
	if (child_pids[0] < 0)
		return ((t_shell_status){.exit_code = -1});
	set_signal_handlers(SIG_IGN, SIG_IGN);
	return ((t_shell_status){.exit_code = wait_for_children(child_pids,
			con.n_children), .should_exit = false, .sigint_received = false});
}
