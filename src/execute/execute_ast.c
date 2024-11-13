/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execute_ast.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/08 20:51:52 by pleander          #+#    #+#             */
/*   Updated: 2024/11/13 18:30:05 by pleander         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <sys/wait.h>
#include "arena.h"
#include "libft.h"
#include "ast.h"
#include "execute.h"
#include "minishell.h"
#include "tokenize.h"

static t_shell_status	execute_pipeline(
		t_ast_node *ast, t_list *env, t_arena arena, int prev_exit);
static	t_shell_status	execute_single_command(
		t_ast_node *ast, t_list *env, t_arena arena, int prev_exit);
static t_shell_status	execute_builtin_cmd(
		t_command_context *con, t_arena arena, int prev_exit);
static int	wait_for_children(int *pid, size_t n_forks);
static int	execute_cmd(t_command_context *con, t_arena arena, int prev_exit);

/**
 * @brief Executes the AST and returns the exit code
 *
 * @param ast AST node
 * @return shell status (exit code + should_exit flag)
 */
t_shell_status	execute_ast(t_ast_node *ast, t_list	*env, t_arena arena, int prev_exit)
{
	t_shell_status	status;

	status.exit_code = 0;
	status.should_exit = false;
	if (ast->type == AST_COMMAND && ast->n_children > 0)
		status = execute_single_command(ast, env, arena, prev_exit);
	else if (ast->type == AST_PIPELINE)
		status = execute_pipeline(ast, env, arena, prev_exit);
	else if (ast->type == AST_LOGICAL)
		status = execute_logicals(ast, env, arena, prev_exit);
	else if (ast->type == AST_PAREN)
		status = execute_ast(ast->children[0], env, arena, prev_exit);
	return (status);
}

static t_shell_status	execute_pipeline(
		t_ast_node *ast, t_list *env, t_arena arena, int prev_exit)
{
	t_command_context	con;
	pid_t				*child_pids;
	t_shell_status		status;

	con.env = env;
	con.n_children = calculate_n_pipes(ast) + 1;
	con.pipes = make_pipes(con.n_children - 1, arena);
	child_pids = arena_alloc(arena, (con.n_children) * sizeof(pid_t));
	if (!child_pids || !con.pipes)
		return ((t_shell_status){.exit_code = -1});
	con.cur_child = 0;
	while (ast->type == AST_PIPELINE || ast->type == AST_COMMAND)
	{
		if (ast->type == AST_PIPELINE)
			con.ast = ast->children[0]; //left child i.e command
		else if (ast->type == AST_COMMAND)
			con.ast = ast;
		child_pids[con.cur_child] = fork();
		if (child_pids[con.cur_child] == 0)
			execute_cmd(&con, arena, prev_exit);
		if (child_pids[con.cur_child] == -1)
			return ((t_shell_status){.exit_code = -1, .should_exit = true}); // Error exit
		if (ast->type == AST_COMMAND)
			break ;
		ast = ast->children[1]; // next pipe opreator
		con.cur_child++;
	}
	close_pipes(con.pipes, con.n_children - 1);
	status.exit_code = wait_for_children(child_pids, con.n_children);
	status.should_exit = false;
	return (status);
}

static	t_shell_status	execute_single_command(
		t_ast_node *ast, t_list *env, t_arena arena, int prev_exit)
{
	t_command_context	con;
	t_shell_status		status;
	pid_t				*child_pids;

	con.ast = ast;
	con.env = env;
	con.pipes = NULL;
	con.cur_child = 0;
	con.n_children = 1;
	if (is_builtin(con.ast->children[0]->token.value))
		return (execute_builtin_cmd(&con, arena, prev_exit));
	child_pids = arena_alloc(arena, (con.n_children) * sizeof(pid_t));
	if (!child_pids)
		return ((t_shell_status){.exit_code = -1});
	child_pids[0] = fork();
	if (child_pids[0] == 0)
		execute_cmd(&con, arena, prev_exit);
	else if (child_pids[0] < 0)
		return ((t_shell_status){.exit_code = -1});
	status.exit_code = wait_for_children(child_pids, con.n_children);
	status.should_exit = false;
	return (status);
}


static int	execute_cmd(t_command_context *con, t_arena arena, int prev_exit)
{
	t_command	cmd;	
	
	cmd.infile_fd = -1; //TODO: Remove duplication between execute_cmd and execute_builtin_cmd
	cmd.outfile_fd = -1;
	if (con->pipes && con->cur_child > 0)
		cmd.infile_fd = con->pipes[con->cur_child - 1][0];
	if (con->pipes && con->cur_child != con->n_children - 1)
		cmd.outfile_fd = con->pipes[con->cur_child][1];
	if (make_command(&cmd, con->ast, con->env, arena) < 0)
	{
		if (DEBUG)
			ft_dprintf(2, "Error making command\n");
		exit (1);
	}
	if (cmd.infile_fd > -1)
		dup2(cmd.infile_fd, STDIN_FILENO);
	if (cmd.outfile_fd > -1)
		dup2(cmd.outfile_fd, STDOUT_FILENO);
	if (con->pipes)
		close_pipes(con->pipes, con->n_children - 1);
	if (DEBUG)
		print_command(&cmd);
	if (is_builtin(cmd.path))
		exit(run_builtin(cmd.path, cmd.args, &con->env, prev_exit).exit_code);
	if (!is_path(cmd.path))
	{
		dprintf(STDERR_FILENO, "%s: %s\n", cmd.path, "command not found");
		exit(EX_NOTFOUND);
	}
	char **env_array = make_raw_env_array(con->env, arena);
	exit (minishell_execve(cmd.path, (char **)cmd.args, env_array));
}

static t_shell_status	execute_builtin_cmd(
		t_command_context *con, t_arena arena, int prev_exit)
{
	t_command		cmd;	
	int				orig_fds[2];
	t_shell_status	status;
	
	orig_fds[0] = -1;
	orig_fds[1] = -1;
	cmd.infile_fd = -1;
	cmd.outfile_fd = -1;
	if (con->pipes && con->cur_child > 0)
		cmd.infile_fd = con->pipes[con->cur_child - 1][0];
	if (con->pipes && con->cur_child != con->n_children - 1)
		cmd.outfile_fd = con->pipes[con->cur_child][1];
	if (make_command(&cmd, con->ast, con->env, arena) < 0)
	{
		if (DEBUG)
			ft_dprintf(2, "Error making command\n");
		return ((t_shell_status){.exit_code = 1});
	}
	if (cmd.infile_fd > -1)
	{
		orig_fds[0] = dup(STDIN_FILENO);
		dup2(cmd.infile_fd, STDIN_FILENO);
	}
	if (cmd.outfile_fd > -1)
	{
		orig_fds[1] = dup(STDOUT_FILENO);
		dup2(cmd.outfile_fd, STDOUT_FILENO);
	}
	if (DEBUG)
		print_command(&cmd);
	status = run_builtin(cmd.path, cmd.args, &con->env, prev_exit);
	// Restore original fds
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
	return (status);
}

static int	wait_for_children(int *pid, size_t n_forks)
{
	int		wstatus;
	int		e_status;
	size_t	i;

	wstatus = 0;
	e_status = 0;
	i = 0;
	while (i < n_forks)
	{
		wstatus = 0;
		if (pid[i] > 0)
		{
			waitpid(pid[i], &wstatus, 0);
			if (WIFEXITED(wstatus))
				e_status = WEXITSTATUS(wstatus);
		}
		else
			e_status = EXIT_FAILURE;
		i++;
	}
	return (e_status);
}
