/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_redir.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 12:48:25 by pleander          #+#    #+#             */
/*   Updated: 2024/11/19 16:17:22 by copireyr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <errno.h>
#include "minishell.h"
#include "signals.h"
#include "libft.h"

static int	handle_redir_in(t_command *cmd, t_ast_node *ast)
{
	close_fd_if_open(&cmd->infile_fd);
	cmd->infile = (char *)ast->children[0]->token.value;
	cmd->infile_fd = open(ast->children[0]->token.value, O_RDONLY);
	if (cmd->infile_fd < 0)
	{
		ft_dprintf(2, "%s: ", NAME);
		perror(cmd->infile);
		return (-1);
	}
	return (0);
}

static int	handle_redir_heredoc(t_command *cmd, t_ast_node *ast)
{
	int		hdoc_pipe[2];
	char	*line;

	set_signal_handlers(SIG_IGN, SIG_DFL);
	close_fd_if_open(&cmd->infile_fd);
	cmd->infile = (char *)ast->children[0]->token.value;
	if (pipe(hdoc_pipe) < 0)
		return (-1 + 0 * ft_dprintf(
				2, "%s: %s: %s", NAME, cmd->infile, strerror(errno)));
	line = readline("> ");
	if (!line)
		return (0);
	while (ft_strncmp(line, cmd->infile, ft_strlen(line) - 1))
	{
		ft_dprintf(hdoc_pipe[1], line);
		free(line);
		line = readline("> ");
		if (!line)
			return (0);
	}
	free(line);
	close(hdoc_pipe[1]);
	cmd->infile_fd = hdoc_pipe[0];
	return (0);
}

static int	handle_redir_out(t_command *cmd, t_ast_node *ast)
{
	close_fd_if_open(&cmd->outfile_fd);
	cmd->outfile = (char *)ast->children[0]->token.value;
	cmd->outfile_fd = open(
			ast->children[0]->token.value, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (cmd->outfile_fd < 0)
	{
		ft_dprintf(2, "%s: ", NAME);
		perror(cmd->outfile);
		return (-1);
	}
	return (0);
}

static int	handle_redir_append(t_command *cmd, t_ast_node *ast)
{
	close_fd_if_open(&cmd->infile_fd);
	cmd->outfile_fd = open(
			ast->children[0]->token.value, O_WRONLY | O_CREAT, 0644);
	if (cmd->outfile_fd < 0)
	{
		ft_dprintf(2, "%s: ", NAME);
		perror(ast->children[0]->token.value);
		return (-1);
	}
	return (0);
}

int	handle_redir(t_command *cmd, t_ast_node *ast)
{
	if (!ast->children)
	{
		ft_dprintf(2, "%s: ambiguous redirect\n", NAME);
		return (-1);
	}
	if (ast->type != AST_REDIR || ast->children[0]->type != AST_WORD)
		return (-1);
	if (ast->token.type == TOK_REDIRECT_IN)
		return (handle_redir_in(cmd, ast));
	else if (ast->token.type == TOK_HEREDOC)
		return (handle_redir_heredoc(cmd, ast));
	else if (ast->token.type == TOK_REDIRECT_OUT)
		return (handle_redir_out(cmd, ast));
	else if (ast->token.type == TOK_APPEND)
		return (handle_redir_append(cmd, ast));
	else
	{
		ft_dprintf(2, "Error: token not supported yet\n");
		return (-1);
	}
	return (0);
}
